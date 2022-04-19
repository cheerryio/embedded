// Copyright 2020-2021 Espressif Systems (Shanghai) CO LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdbool.h>
#include <string.h>
#include <sys/queue.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

#include "led_indicator.h"
#include "esp_log.h"

static const char *TAG = "led_indicator";

#define LED_INDICATOR_CHECK(a, str, ret) if(!(a)) { \
        ESP_LOGE(TAG,"%s:%d (%s):%s", __FILE__, __LINE__, __FUNCTION__, str); \
        return (ret); \
    }

#define LED_INDICATOR_CHECK_GOTO(a, str, lable) if(!(a)) { \
        ESP_LOGE(TAG,"%s:%d (%s):%s", __FILE__, __LINE__, __FUNCTION__, str); \
        goto lable; \
    }

#define NULL_ACTIVE_BLINK -1

#define PIN_NUM_WR 22
#define PIN_NUM_DATA 23
#define PIN_NUM_CS 21

spi_device_handle_t spi;
humitemp_lcd_handle_t humitemp_lcd_handle;

/*********************************** Config Blink List in Different Conditions ***********************************/
/**
 * @brief connecting to AP (or Cloud)
 * 
 */
static const blink_step_t wifi_notconnected[] = {
    {SIGNAL_LEVEL0,LED_STATE_ON,200},
    {BLINK_LOOP,0,0},
};

/**
 * @brief connected to AP (or Cloud) succeed
 * 
 */
static const blink_step_t wifi_connected[] = {
    {SIGNAL_LEVEL4,LED_STATE_ON,200},
    {BLINK_LOOP,0, 0},
};

/**
 * @brief reconnecting to AP (or Cloud), if lose connection 
 * 
 */
static const blink_step_t wifi_connecting[] = {
    {SIGNAL_LEVEL0,LED_STATE_ON,300},
    {SIGNAL_LEVEL1,LED_STATE_ON,300},
    {SIGNAL_LEVEL2,LED_STATE_ON,300},
    {SIGNAL_LEVEL3,LED_STATE_ON,300},
    {SIGNAL_LEVEL4,LED_STATE_ON,300},
    {BLINK_LOOP,0,0},
}; //offline

/**
 * @brief updating software
 * 
 */
static const blink_step_t awss_press[] = {
    {SIGNAL_LEVEL0,LED_STATE_ON,200},
    {SIGNAL_LEVEL1,LED_STATE_ON,200},
    {SIGNAL_LEVEL2,LED_STATE_ON,200},
    {SIGNAL_LEVEL3,LED_STATE_ON,200},
    {SIGNAL_LEVEL4,LED_STATE_ON,200},
    {BLINK_LOOP,0,0},
};

/**
 * @brief restoring factory settings
 * 
 */
static const blink_step_t awss_comm[] = {
    {SIGNAL_LEVEL0,LED_STATE_ON,300},
    {SIGNAL_LEVEL1,LED_STATE_ON,250},
    {SIGNAL_LEVEL2,LED_STATE_ON,200},
    {SIGNAL_LEVEL3,LED_STATE_ON,150},
    {SIGNAL_LEVEL4,LED_STATE_ON,100},
    {BLINK_LOOP,0,0},
};

/**
 * @brief provisioning
 * 
 */
static const blink_step_t modbus_broken[] = {
    {BLINK_COMM,LED_STATE_OFF, 500},
    {BLINK_LOOP, 0, 0},
};

/**
 * @brief provision done
 * 
 */
static const blink_step_t modbus_connected[] = {
    {BLINK_COMM,LED_STATE_ON, 500},
    {BLINK_LOOP, 0, 0},
};

static const blink_step_t modbus_comm[]={
    {BLINK_COMM,LED_STATE_ON, 200},
    {BLINK_COMM,LED_STATE_OFF,200},
    {BLINK_LOOP, 0, 0},
};

/**
 * @brief led indicator blink lists, the index like BLINK_FACTORY_RESET defined the priority of the blink
 * 
 */
blink_step_t const * led_indicator_blink_lists[] = {
    [BLINK_WIFI_NOTCONNECTED] = wifi_notconnected,
    [BLINK_WIFI_CONNECTED] = wifi_connected,
    [BLINK_WIFI_CONNECTING] = wifi_connecting,
    [BLINK_AWSS_PRESS] = awss_press,
    [BLINK_AWSS_COMM] = awss_comm,
    [BLINK_MODBUS_BROKEN] = modbus_broken,
    [BLINK_MODBUS_CONNECTED] = modbus_connected,
    [BLINK_MODBUS_COMM] = modbus_comm,
    [BLINK_MAX] = NULL,
};

/* Led blink_steps handling machine implementation */
#define BLINK_LIST_NUM (sizeof(led_indicator_blink_lists)/sizeof(led_indicator_blink_lists[0]))

/**
 * @brief led indicator object
 * 
 */
typedef struct {
    bool off_level; /*!< gpio level during led turn off */
    int io_num; /*!< gpio number of the led indicator */
    led_indicator_mode_t mode; /*!< led work mode, eg. gpio or pwm mode */
    int active_blink; /*!< active blink list*/
    int *p_blink_steps; /*!< stage of each blink list */
    SemaphoreHandle_t mutex; /*!< mutex to achive thread-safe */
    TimerHandle_t h_timer; /*!< led timmer handle, invalid if works in pwm mode */
} led_indicator_t;

typedef struct led_indicator_slist_t{
    SLIST_ENTRY(led_indicator_slist_t) next;
    led_indicator_t *p_led_indicator;
} led_indicator_slist_t;

static SLIST_HEAD( led_indicator_head_t, led_indicator_slist_t) s_led_indicator_slist_head = SLIST_HEAD_INITIALIZER(s_led_indicator_slist_head);

static esp_err_t led_indicator_add_node(led_indicator_t *p_led_indicator)
{
    LED_INDICATOR_CHECK(p_led_indicator != NULL, "pointer can not be NULL", ESP_ERR_INVALID_ARG);
    led_indicator_slist_t *node = calloc(1, sizeof(led_indicator_slist_t));
    LED_INDICATOR_CHECK(node != NULL, "calloc node failed", ESP_ERR_NO_MEM);
    node->p_led_indicator = p_led_indicator;
    SLIST_INSERT_HEAD(&s_led_indicator_slist_head, node, next);
    return ESP_OK;
}

static esp_err_t led_indicator_remove_node(led_indicator_t *p_led_indicator)
{
    LED_INDICATOR_CHECK(p_led_indicator != NULL, "pointer can not be NULL", ESP_ERR_INVALID_ARG);
    led_indicator_slist_t *node;
    SLIST_FOREACH(node, &s_led_indicator_slist_head, next) {
        if (node->p_led_indicator == p_led_indicator) {
            SLIST_REMOVE(&s_led_indicator_slist_head, node, led_indicator_slist_t, next);
            free(node);
            break;
        }
    }
    return ESP_OK;
}

/**
 * @brief switch to the first high priority incomplete blink steps
 * 
 * @param p_led_indicator pointer to led indicator
 */
static void blink_list_switch(led_indicator_t *p_led_indicator)
{
    p_led_indicator->active_blink = NULL_ACTIVE_BLINK; //stop active blink
    for(size_t index = 0; index < BLINK_LIST_NUM; index ++) //find the first incomplete blink
    {
        if (p_led_indicator->p_blink_steps[index] != BLINK_STOP)
        {
            p_led_indicator->active_blink = index;
            break;
        }
    }
}

/**
 * @brief timmer callback to control led and counter steps
 * 
 * @param xTimer handle of the timmer instance
 */
static void blink_list_runner(xTimerHandle xTimer)
{
    led_indicator_t * p_led_indicator = (led_indicator_t *)pvTimerGetTimerID(xTimer);
    bool leave = false;

    while(!leave) {

        if (p_led_indicator->active_blink == NULL_ACTIVE_BLINK)
        return;

        int active_blink = p_led_indicator->active_blink;
        int active_step = p_led_indicator->p_blink_steps[active_blink];
        const blink_step_t *p_blink_step = &led_indicator_blink_lists[active_blink][active_step];

        p_led_indicator->p_blink_steps[active_blink] += 1;

        if (pdFALSE == xSemaphoreTake(p_led_indicator->mutex, (100 / portTICK_RATE_MS))) {
            ESP_LOGW(TAG, "blinks runner blockTime expired, try repairing...");
            xTimerChangePeriod(p_led_indicator->h_timer, (100 / portTICK_RATE_MS), 0);
            xTimerStart(p_led_indicator->h_timer, 0);
            break;
        }

        switch(p_blink_step->type) {
            case BLINK_LOOP:
                p_led_indicator->p_blink_steps[active_blink] = 0;
                break;
            case BLINK_STOP:
                p_led_indicator->p_blink_steps[active_blink] = BLINK_STOP;
                blink_list_switch(p_led_indicator);
                break;
            case SIGNAL_LEVEL0:
                humitemp_lcd_set_s1(humitemp_lcd_handle,1);
                humitemp_lcd_set_s2(humitemp_lcd_handle,0);
                humitemp_lcd_set_s3(humitemp_lcd_handle,0);
                humitemp_lcd_set_s4(humitemp_lcd_handle,0);
                humitemp_lcd_set_s5(humitemp_lcd_handle,0);
                humitemp_lcd_flush(humitemp_lcd_handle);
                xTimerChangePeriod(p_led_indicator->h_timer, (p_blink_step->hold_time_ms / portTICK_RATE_MS), 0);
                xTimerStart(p_led_indicator->h_timer, 0);
                leave=true;
                break;
            case SIGNAL_LEVEL1:
                humitemp_lcd_set_s2(humitemp_lcd_handle,1);
                humitemp_lcd_flush(humitemp_lcd_handle);
                xTimerChangePeriod(p_led_indicator->h_timer, (p_blink_step->hold_time_ms / portTICK_RATE_MS), 0);
                xTimerStart(p_led_indicator->h_timer, 0);
                leave=true;
                break;
            case SIGNAL_LEVEL2:
                humitemp_lcd_set_s3(humitemp_lcd_handle,1);
                humitemp_lcd_flush(humitemp_lcd_handle);
                xTimerChangePeriod(p_led_indicator->h_timer, (p_blink_step->hold_time_ms / portTICK_RATE_MS), 0);
                xTimerStart(p_led_indicator->h_timer, 0);
                leave=true;
                break;
            case SIGNAL_LEVEL3:
                humitemp_lcd_set_s4(humitemp_lcd_handle,1);
                humitemp_lcd_flush(humitemp_lcd_handle);
                xTimerChangePeriod(p_led_indicator->h_timer, (p_blink_step->hold_time_ms / portTICK_RATE_MS), 0);
                xTimerStart(p_led_indicator->h_timer, 0);
                leave=true;
                break;
            case SIGNAL_LEVEL4:
                humitemp_lcd_set_s1(humitemp_lcd_handle,1);
                humitemp_lcd_set_s2(humitemp_lcd_handle,1);
                humitemp_lcd_set_s3(humitemp_lcd_handle,1);
                humitemp_lcd_set_s4(humitemp_lcd_handle,1);
                humitemp_lcd_set_s5(humitemp_lcd_handle,1);
                humitemp_lcd_flush(humitemp_lcd_handle);
                xTimerChangePeriod(p_led_indicator->h_timer, (p_blink_step->hold_time_ms / portTICK_RATE_MS), 0);
                xTimerStart(p_led_indicator->h_timer, 0);
                leave=true;
                break;
            case BLINK_COMM:
                humitemp_lcd_set_com(humitemp_lcd_handle,p_blink_step->on_off);
                humitemp_lcd_flush(humitemp_lcd_handle);
                if (p_blink_step->hold_time_ms == 0) break;
                xTimerChangePeriod(p_led_indicator->h_timer, (p_blink_step->hold_time_ms / portTICK_RATE_MS), 0);
                xTimerStart(p_led_indicator->h_timer, 0);
                leave=true;
                break;
            default:
                assert(false && "invalid state");
                break;
        }
        xSemaphoreGive(p_led_indicator->mutex);
    }
}

led_indicator_handle_t led_indicator_create(int num,const led_indicator_config_t* config)
{
    LED_INDICATOR_CHECK(config != NULL, "invalid config pointer", NULL);
    char timmer_name[16] = {'\0'};
    snprintf(timmer_name, sizeof(timmer_name) - 1, "%s%02x", "led_tmr_",num);
    led_indicator_t *p_led_indicator = (led_indicator_t *)calloc(1, sizeof(led_indicator_t));
    LED_INDICATOR_CHECK(p_led_indicator != NULL, "calloc indicator memory failed", NULL);
    p_led_indicator->off_level = config->off_level;
    p_led_indicator->active_blink = NULL_ACTIVE_BLINK;
    p_led_indicator->p_blink_steps = (int *)calloc(BLINK_LIST_NUM, sizeof(int));
    LED_INDICATOR_CHECK_GOTO(p_led_indicator->p_blink_steps != NULL, "calloc blink_steps memory failed", cleanup_indicator);
    p_led_indicator->mutex = xSemaphoreCreateMutex();
    LED_INDICATOR_CHECK_GOTO(p_led_indicator->mutex != NULL, "create mutex failed", cleanup_indicator_blinkstep);
    
    for(size_t j = 0; j < BLINK_LIST_NUM; j++) {
        *(p_led_indicator->p_blink_steps + j) = BLINK_STOP;
    }
    
    p_led_indicator->h_timer = xTimerCreate(timmer_name, (100 / portTICK_RATE_MS), pdFALSE, (void *)p_led_indicator, blink_list_runner);
    LED_INDICATOR_CHECK_GOTO(p_led_indicator->h_timer != NULL, "led timmer create failed", cleanup_all);

    led_indicator_add_node(p_led_indicator);
    return (led_indicator_handle_t)p_led_indicator;

cleanup_indicator:
    free(p_led_indicator);
    return NULL;
cleanup_indicator_blinkstep:
    free(p_led_indicator->p_blink_steps);
    free(p_led_indicator);
    return NULL;
cleanup_all:
    vSemaphoreDelete(p_led_indicator->mutex);
    free(p_led_indicator->p_blink_steps);
    free(p_led_indicator);
    return NULL;
}

led_indicator_handle_t led_indicator_get_handle(int io_num)
{
    led_indicator_slist_t *node;
    SLIST_FOREACH(node, &s_led_indicator_slist_head, next) {
        if (node->p_led_indicator->io_num == io_num) {
            return (led_indicator_handle_t)(node->p_led_indicator);
        }
    }
    return NULL;
}

esp_err_t led_indicator_delete(led_indicator_handle_t* p_handle)
{
    LED_INDICATOR_CHECK(p_handle != NULL && *p_handle != NULL, "invalid p_handle", ESP_ERR_INVALID_ARG);
    led_indicator_t *p_led_indicator = (led_indicator_t *)(*p_handle);
    xSemaphoreTake(p_led_indicator->mutex, portMAX_DELAY);

    BaseType_t ret = xTimerDelete(p_led_indicator->h_timer, portMAX_DELAY);
    LED_INDICATOR_CHECK(ret == pdPASS, "led timmer delete failed", ESP_FAIL);

    led_indicator_remove_node(p_led_indicator);
    vSemaphoreDelete(p_led_indicator->mutex);
    free(p_led_indicator->p_blink_steps);
    free(*p_handle);
    *p_handle = NULL;
    return ESP_OK;
}

esp_err_t led_indicator_start(led_indicator_handle_t handle, led_indicator_blink_type_t blink_type)
{
    LED_INDICATOR_CHECK(handle != NULL && blink_type >= 0 && blink_type < BLINK_MAX, "invalid p_handle", ESP_ERR_INVALID_ARG);
    LED_INDICATOR_CHECK(led_indicator_blink_lists[blink_type] != NULL, "undefined blink_type", ESP_ERR_INVALID_ARG);
    led_indicator_t *p_led_indicator = (led_indicator_t *)handle;
    xSemaphoreTake(p_led_indicator->mutex, portMAX_DELAY);
    p_led_indicator->p_blink_steps[blink_type] = 0;
    blink_list_switch(p_led_indicator);
    xSemaphoreGive(p_led_indicator->mutex);

    if(p_led_indicator->active_blink == blink_type) { //re-run from first step
        blink_list_runner(p_led_indicator->h_timer);
    }

    return ESP_OK;
}

esp_err_t led_indicator_stop(led_indicator_handle_t handle, led_indicator_blink_type_t blink_type)
{
    LED_INDICATOR_CHECK(handle != NULL && blink_type >= 0 && blink_type < BLINK_MAX, "invalid p_handle", ESP_ERR_INVALID_ARG);
    LED_INDICATOR_CHECK(led_indicator_blink_lists[blink_type] != NULL, "undefined blink_type", ESP_ERR_INVALID_ARG);
    led_indicator_t *p_led_indicator = (led_indicator_t *)handle;
    xSemaphoreTake(p_led_indicator->mutex, portMAX_DELAY);
    p_led_indicator->p_blink_steps[blink_type] = BLINK_STOP;
    blink_list_switch(p_led_indicator); //stop and swith to next blink steps
    xSemaphoreGive(p_led_indicator->mutex);

    if(p_led_indicator->active_blink == blink_type) { //re-run from first step
        blink_list_runner(p_led_indicator->h_timer);
    }

    return ESP_OK;
}
