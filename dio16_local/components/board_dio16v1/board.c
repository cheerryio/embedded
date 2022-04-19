// Copyright 2021-2022 Forchine Technology (Wuhan)  LTD
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

#include <stdio.h>
#include "board.h"
#include "esp_log.h"
#include "iot_button.h"
#include "xl9555.h"
#include "led_indicator.h"
static const char *TAG = "Board";
static bool s_board_is_init = false;

#define BOARD_CHECK(a, str, ret) if(!(a)) { \
        ESP_LOGE(TAG,"%s:%d (%s):%s", __FILE__, __LINE__, __FUNCTION__, str); \
        return (ret); \
    }

/****Private board level API ****/

static i2c_bus_handle_t s_i2c2_bus_handle = NULL;
static xl9555_handle_t s_xl9555_bank0_handle = NULL;
static xl9555_handle_t s_xl9555_bank1_handle = NULL;
static button_handle_t provision_key = NULL;

static led_indicator_handle_t led_handle_485_r = NULL;
static led_indicator_handle_t led_handle_485_g = NULL;
static led_indicator_handle_t led_handle_net_r = NULL;
static led_indicator_handle_t led_handle_net_g = NULL;
/**
 * @brief i2c master initialization
 */



static esp_err_t board_i2c_bus_init(void)
{

    i2c_config_t i2c2_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = BOARD_IO_I2C2_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = BOARD_IO_I2C2_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = BOARD_I2C_SPEED,
    };    
    
    s_i2c2_bus_handle = i2c_bus_create(I2C_NUM_1, &i2c2_conf);

    BOARD_CHECK(s_i2c2_bus_handle != NULL, "i2c_bus creat failed", ESP_FAIL);

    return ESP_OK;
}

static esp_err_t board_i2c_bus_deinit(void)
{
    if (s_i2c2_bus_handle != NULL) {
        i2c_bus_delete(&s_i2c2_bus_handle);
        if (s_i2c2_bus_handle != NULL) {
            return ESP_FAIL;
        }
    }
    return ESP_OK;
}


static esp_err_t board_xl9555_init()
{
    
    s_xl9555_bank0_handle = xl9555_create(s_i2c2_bus_handle, BOARD_I2C_ADDR_XL9555_BANK0);
    s_xl9555_bank1_handle = xl9555_create(s_i2c2_bus_handle, BOARD_I2C_ADDR_XL9555_BANK1);

    uint8_t init_out = 0;

    xl9555_set_dir(s_xl9555_bank0_handle,0,XL9555_BANK0_P0_DIR);
    xl9555_set_output(s_xl9555_bank0_handle,1,&init_out);
    xl9555_set_dir(s_xl9555_bank0_handle,1,XL9555_BANK0_P1_DIR);
    
    init_out = 0;
    xl9555_set_dir(s_xl9555_bank1_handle,0,XL9555_BANK1_P0_DIR);
    xl9555_set_output(s_xl9555_bank1_handle,1,&init_out);
    xl9555_set_dir(s_xl9555_bank1_handle,1,XL9555_BANK1_P1_DIR);

    return ESP_OK;
}


static esp_err_t board_button_init(void)
{
    button_config_t cfg = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num = BOARD_IO_BUTTON_A,
            .active_level = 0,
        },
    };
    provision_key = iot_button_create(&cfg);
    return ESP_OK;
}


esp_err_t iot_board_init(void)
{
    if(s_board_is_init) {
        return ESP_OK;
    }
    int ret;

    ret = board_i2c_bus_init();
    BOARD_CHECK(ret == ESP_OK, "i2c init failed", ret);

    uint8_t scan_buff[5];
    uint8_t i2c_num = i2c_bus_scan(s_i2c2_bus_handle, scan_buff, 5);
    while(i2c_num)
    {
        ESP_LOGI(TAG,"i2c scan found %x.", scan_buff[i2c_num-1]);
        i2c_num -- ;
    }

    board_xl9555_init();
    
    board_button_init();

    led_indicator_config_t config = {
        .off_level = 1,
        .mode = LED_GPIO_MODE,
    };

    led_handle_485_r = led_indicator_create(BOARD_IO_LED_485_R, &config);
    led_handle_485_g = led_indicator_create(BOARD_IO_LED_485_G, &config);
    led_handle_net_r = led_indicator_create(BOARD_IO_LED_NET_R, &config);
    led_handle_net_g = led_indicator_create(BOARD_IO_LED_NET_G, &config);

    s_board_is_init = true;
    ESP_LOGI(TAG,"Board Info: %s", iot_board_get_info());
    ESP_LOGI(TAG,"Board Init Done ...");
    return ESP_OK;
}

esp_err_t iot_board_deinit(void)
{  
    if(s_board_is_init) {
        xl9555_delete(&s_xl9555_bank0_handle);
        xl9555_delete(&s_xl9555_bank1_handle);
        board_i2c_bus_deinit();
        iot_button_delete(provision_key);
        led_indicator_delete(&led_handle_485_r);
        led_indicator_delete(&led_handle_485_g);
        led_indicator_delete(&led_handle_net_r);
        led_indicator_delete(&led_handle_net_g);
        s_board_is_init = false;
        ESP_LOGI(TAG,"Board DeInit Done ...");
        return ESP_OK;
    }
    else
    {
        ESP_LOGE(TAG,"Board DeInit Not init.");
        return ESP_ERR_NOT_FOUND;
    }
        
}

bool iot_board_is_init(void)
{
    return s_board_is_init;
}

board_res_handle_t iot_board_get_handle(board_res_id_t id)
{
    board_res_handle_t handle;
    switch (id)
    {
    case BOARD_XL9555_BANK0_ID:
        handle = (board_res_handle_t)s_xl9555_bank0_handle;
        break;
    case BOARD_XL9555_BANK1_ID:
        handle = (board_res_handle_t)s_xl9555_bank1_handle;
        break;
    case BOARD_BUTTON_ID:
        handle = (board_res_handle_t)provision_key;
        break;
    case BOARD_LED_485_R:
        handle = (board_res_handle_t)led_handle_485_r;
        break;
    case BOARD_LED_485_G:
        handle = (board_res_handle_t)led_handle_485_g;
        break;
    case BOARD_LED_NET_R:
        handle = (board_res_handle_t)led_handle_net_r;
        break;
    case BOARD_LED_NET_G:
        handle = (board_res_handle_t)led_handle_net_g;
        break;
    default:
        handle = NULL;
        break;
    }
    return handle;
}

char* iot_board_get_info()
{
    static char* info = BOARD_NAME;
    return info;
}
