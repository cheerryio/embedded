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

#include "esp_system.h"
#include "esp_log.h"
#include "led_indicator.h"
#include "unity.h"

#define LED_IO_NUM_0    23
#define LED_IO_NUM_1    18
#define LED_IO_NUM_2    5

spi_device_handle_t spi;
humitemp_lcd_handle_t humitemp_lcd_handle;

static const char* TAG="led_indicator";

static led_indicator_handle_t led_handle_0 = NULL;
static led_indicator_handle_t led_handle_1 = NULL;

static void humitemp_lcd_setup(){
    spi_setup(&spi);

    humitemp_lcd_create_handle(spi,&humitemp_lcd_handle);
    humitemp_lcd_init(humitemp_lcd_handle);
    humitemp_lcd_clear_all(humitemp_lcd_handle);
    humitemp_lcd_flush(humitemp_lcd_handle);
}

static void humitemp_lcd_teardown(){
    humitemp_lcd_delete_handle(humitemp_lcd_handle);
}

static void led_indicator_init()
{
    led_indicator_config_t config = {
        .off_level = 0,
    };

    led_handle_0 = led_indicator_create(0,&config);
    led_handle_1 = led_indicator_create(1,&config);
    TEST_ASSERT_NOT_NULL(led_handle_0);
    TEST_ASSERT_NOT_NULL(led_handle_1);
}

static void led_indicator_deinit()
{
    esp_err_t ret = led_indicator_delete(&led_handle_0);
    TEST_ASSERT(ret == ESP_OK);
    TEST_ASSERT_NULL(led_handle_0);
    ret = led_indicator_delete(&led_handle_1);
    TEST_ASSERT(ret == ESP_OK);
    TEST_ASSERT_NULL(led_handle_1);
}

static void led_indicator_test_signal()
{
    esp_err_t ret=ESP_OK;

    ESP_LOGI(TAG, "not connected.....");
    ret = led_indicator_start(led_handle_0, BLINK_WIFI_NOTCONNECTED);
    TEST_ASSERT(ret == ESP_OK);
    vTaskDelay(4000 / portTICK_RATE_MS);
    ret = led_indicator_stop(led_handle_0, BLINK_WIFI_NOTCONNECTED);
    TEST_ASSERT(ret == ESP_OK);
    vTaskDelay(1000 / portTICK_RATE_MS);

    ESP_LOGI(TAG, "connected.....");
    ret = led_indicator_start(led_handle_0, BLINK_WIFI_CONNECTED);
    TEST_ASSERT(ret == ESP_OK);
    vTaskDelay(4000 / portTICK_RATE_MS);
    ret = led_indicator_stop(led_handle_0, BLINK_WIFI_CONNECTED);
    TEST_ASSERT(ret == ESP_OK);
    vTaskDelay(1000 / portTICK_RATE_MS);

    ESP_LOGI(TAG, "connecting.....");
    ret = led_indicator_start(led_handle_0, BLINK_WIFI_CONNECTING);
    TEST_ASSERT(ret == ESP_OK);
    vTaskDelay(4000 / portTICK_RATE_MS);
    ret = led_indicator_stop(led_handle_0, BLINK_WIFI_CONNECTING);
    TEST_ASSERT(ret == ESP_OK);
    vTaskDelay(1000 / portTICK_RATE_MS);

    ESP_LOGI(TAG, "awss press.....");
    ret = led_indicator_start(led_handle_0, BLINK_AWSS_PRESS);
    TEST_ASSERT(ret == ESP_OK);
    vTaskDelay(4000 / portTICK_RATE_MS);
    ret = led_indicator_stop(led_handle_0, BLINK_AWSS_PRESS);
    TEST_ASSERT(ret == ESP_OK);
    vTaskDelay(1000 / portTICK_RATE_MS);

    ESP_LOGI(TAG, "awss com.....");
    ret = led_indicator_start(led_handle_0, BLINK_AWSS_COMM);
    TEST_ASSERT(ret == ESP_OK);
    vTaskDelay(4000 / portTICK_RATE_MS);
    ret = led_indicator_stop(led_handle_0, BLINK_AWSS_COMM);
    TEST_ASSERT(ret == ESP_OK);
    vTaskDelay(1000 / portTICK_RATE_MS);

    ESP_LOGI(TAG, "modbus broken.....");
    ret = led_indicator_start(led_handle_1, BLINK_MODBUS_BROKEN);
    TEST_ASSERT(ret == ESP_OK);
    vTaskDelay(4000 / portTICK_RATE_MS);
    ret = led_indicator_stop(led_handle_1, BLINK_MODBUS_BROKEN);
    TEST_ASSERT(ret == ESP_OK);
    vTaskDelay(1000 / portTICK_RATE_MS);

    ESP_LOGI(TAG, "modbus connected.....");
    ret = led_indicator_start(led_handle_1, BLINK_MODBUS_CONNECTED);
    TEST_ASSERT(ret == ESP_OK);
    vTaskDelay(4000 / portTICK_RATE_MS);
    ret = led_indicator_stop(led_handle_1, BLINK_MODBUS_CONNECTED);
    TEST_ASSERT(ret == ESP_OK);
    vTaskDelay(1000 / portTICK_RATE_MS);

    ESP_LOGI(TAG, "modbus comm.....");
    ret = led_indicator_start(led_handle_1, BLINK_MODBUS_COMM);
    TEST_ASSERT(ret == ESP_OK);
    vTaskDelay(4000 / portTICK_RATE_MS);
    ret = led_indicator_stop(led_handle_1, BLINK_MODBUS_COMM);
    TEST_ASSERT(ret == ESP_OK);
    vTaskDelay(1000 / portTICK_RATE_MS);

    ESP_LOGI(TAG, "test all condition done.....");
}

TEST_CASE("hello","[led][indicator]")
{
    printf("hello\n\r");
}

TEST_CASE("blink test all in order", "[led][indicator]")
{
    humitemp_lcd_setup();
    led_indicator_init();

    led_indicator_test_signal();

    led_indicator_deinit();
    humitemp_lcd_teardown();
}