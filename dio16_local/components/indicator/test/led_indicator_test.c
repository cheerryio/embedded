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
#include "board.h"
#define TAG "led indicator"

static led_indicator_handle_t    h_485_red ;
static led_indicator_handle_t    h_485_green ;
static led_indicator_handle_t    h_net_red ;
static led_indicator_handle_t    h_net_green;

static void test_setup(void)
{
    (iot_board_init());
    h_485_red = (led_indicator_handle_t)iot_board_get_handle(BOARD_LED_485_R);
    h_485_green = (led_indicator_handle_t)iot_board_get_handle(BOARD_LED_485_G);
    h_net_red = (led_indicator_handle_t)iot_board_get_handle(BOARD_LED_NET_R);
    h_net_green = (led_indicator_handle_t)iot_board_get_handle(BOARD_LED_NET_G);
}

static void test_teardown(void)
{
    (iot_board_deinit());
}



TEST_CASE("all led", "[led indicate]")
{
    test_setup();

    led_indicator_start(h_net_green, BLINK_MODBUS_COMM );

    vTaskDelay(pdMS_TO_TICKS(1000));
    led_indicator_stop(h_net_green,BLINK_MODBUS_COMM);



    led_indicator_start(h_net_red, BLINK_MODBUS_COMM );

    vTaskDelay(pdMS_TO_TICKS(1000));
    led_indicator_stop(h_net_red,BLINK_MODBUS_COMM);


    led_indicator_start(h_485_green, BLINK_MODBUS_COMM );

    vTaskDelay(pdMS_TO_TICKS(1000));
    led_indicator_stop(h_485_green,BLINK_MODBUS_COMM);


    led_indicator_start(h_485_red, BLINK_MODBUS_COMM );

    vTaskDelay(pdMS_TO_TICKS(1000));
    led_indicator_stop(h_485_red,BLINK_MODBUS_COMM);

    test_teardown();
}

TEST_CASE("wifi noconfig", "[led indicate]")
{
    test_setup();

    led_indicator_start(h_net_green, BLINK_WIFI_NOCONFIG );

    vTaskDelay(pdMS_TO_TICKS(5000));
    led_indicator_stop(h_net_green,BLINK_WIFI_NOCONFIG);

    test_teardown();
}

TEST_CASE("wifi all ", "[led indicate]")
{
    test_setup();
    for(int i = 0;i<BLINK_MAX;i++)
    {
        led_indicator_start(h_net_green, i );

        vTaskDelay(pdMS_TO_TICKS(5000));
        led_indicator_stop(h_net_green,i);
    }
    test_teardown();
}

TEST_CASE("modbus_comm ", "[led indicate]")
{
    test_setup();
    for(int i = 0;i<10;i++)
    {
        led_indicator_start(h_485_green, BLINK_MODBUS_COMM );

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    test_teardown();
}
