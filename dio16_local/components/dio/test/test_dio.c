// Copyright 2015-2017 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/unistd.h>
#include "unity.h"
#include "test_utils.h"
#include "esp_log.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_partition.h"
#include "esp_rom_sys.h"

#include "dio.h"
#include "board.h"
static const char *TAG = "test_dio";
static void test_setup(void)
{
    TEST_ESP_OK(iot_board_init());
    TEST_ESP_OK(dio_device_init());
}

static void test_teardown(void)
{
    TEST_ESP_OK(dio_device_deinit());
    TEST_ESP_OK(iot_board_deinit());
}

TEST_CASE("get input16", "[dio16]")
{
    test_setup();
    for(int i=0;i<20;i++)
    {
        uint16_t value = 0;
        TEST_ESP_OK(dio_get_input16(&value));
        ESP_LOGI(TAG, "INPUT 0~15 result %x.\n",value );
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    test_teardown();
}

TEST_CASE("set and get output16","[dio16]")
{
    test_setup();
    uint16_t value;
    for(int i=0;i<15;i++)
    {
        value = 1<<i;
        TEST_ESP_OK(dio_set_output16(value));
        vTaskDelay(pdMS_TO_TICKS(100));
        uint16_t v_get = 0;
        TEST_ESP_OK(dio_get_output16(&v_get));
        TEST_ASSERT_EQUAL(value,v_get);
        vTaskDelay(pdMS_TO_TICKS(400));
    }
    TEST_ESP_OK(dio_set_output16(0));
    test_teardown();
}

TEST_CASE("get input bit","[dio16]")
{
    bool value = 0;
    test_setup();
    TEST_ESP_ERR(DIO_PIN_OUT_RANGE,dio_get_din(16,&value));
    for(uint8_t i=0;i<16;i++)
    {
        TEST_ESP_OK(dio_get_din(i,&value));
        ESP_LOGI(TAG, "INPUT %d result %x.\n",i,value );
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    TEST_ESP_ERR(DIO_PIN_OUT_RANGE,dio_get_din(17,&value));
    test_teardown();
}

TEST_CASE("set and get out bit","[dio16]")
{
    bool value = 0;
    test_setup();
    
    TEST_ESP_ERR(DIO_PIN_OUT_RANGE,dio_get_dout(16,&value));
    for(uint8_t i=0;i<16;i++)
    {
        bool t_v = false;
        TEST_ESP_OK(dio_set_dout(i,true));
        vTaskDelay(pdMS_TO_TICKS(500));
        TEST_ESP_OK(dio_get_dout(i,&t_v));
        TEST_ASSERT(t_v);
        vTaskDelay(pdMS_TO_TICKS(500));
        TEST_ESP_OK(dio_set_dout(i,false));
    }
    TEST_ESP_ERR(DIO_PIN_OUT_RANGE,dio_set_dout(17,true));
    test_teardown();
}