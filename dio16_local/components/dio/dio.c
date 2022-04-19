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
#include "dio.h"
#include "xl9555.h"
#include "board.h"
#include "esp_log.h"

xl9555_handle_t dio_bank0_handle, dio_bank1_handle;

esp_err_t dio_device_init(void)
{
    dio_bank0_handle =  (xl9555_handle_t) iot_board_get_handle(BOARD_XL9555_BANK0_ID);
    dio_bank1_handle =  (xl9555_handle_t) iot_board_get_handle(BOARD_XL9555_BANK1_ID);   
    return ESP_OK;
}


esp_err_t dio_device_deinit(void)
{
    return ESP_OK;
}


int dio_set_dout(uint8_t pin, bool value)
{
    int res = 0;
    uint16_t dout16;
    if(pin>15)
    {
        res|=DIO_PIN_OUT_RANGE;
        ESP_LOGE("dio", "set dout range error.");   
        return res;
    }

    res = dio_get_output16(&dout16);
    if(value){
        dout16 |= BIT(pin);
    }
    else{
        dout16 &= ~BIT(pin);
    }
    res = dio_set_output16(dout16);
    return ESP_OK;
}


int dio_get_dout(uint8_t pin, bool*p_value)
{
    int res = 0;
    uint16_t dout16;
    if(pin>15)
    {
        res|=DIO_PIN_OUT_RANGE;
        ESP_LOGE("dio", "get dout range error.");     
        return res;
    }

    res = dio_get_output16(&dout16);
    *p_value = dout16 & BIT(pin);
    return res;
}


int dio_get_din(uint8_t pin, bool*p_value)
{
    int res = 0;
    uint16_t din16;
    if(pin>15)
    {
        res|=DIO_PIN_OUT_RANGE;
        ESP_LOGE("dio", "get din range error.");     
        return res;
    }

    res = dio_get_input16(&din16);
    *p_value = din16 & BIT(pin);
    return res;
}


int dio_get_input16( uint16_t* p_value)
{
    int ret = DIO_OK;
    esp_err_t err;
    err = xl9555_get_input(dio_bank0_handle, 0, (uint8_t*)p_value);
    if(err != ESP_OK)
    {
        ret |= DIO_BANK0_READ_ERROR;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
    err = xl9555_get_input(dio_bank1_handle, 0, (uint8_t*)p_value+1 );
    if(err != ESP_OK)
    {
        ret |= DIO_BANK1_READ_ERROR;
    }
    return ret;
}


int dio_get_output16( uint16_t* p_value)
{
    int ret = DIO_OK;
    esp_err_t err;
    err = xl9555_get_output(dio_bank0_handle, 1, (uint8_t*)p_value);
    if(err != ESP_OK)
    {
        ret |= DIO_BANK0_READ_ERROR;
    }

    err = xl9555_get_output(dio_bank1_handle, 1, (uint8_t*)p_value+1);
    if(err != ESP_OK)
    {
        ret |= DIO_BANK1_READ_ERROR;
    }
    return ret;
}

int dio_set_output16( uint16_t value)
{
    int ret = DIO_OK;
    esp_err_t err;
    err = xl9555_set_output(dio_bank0_handle, 1, (uint8_t*)&value);
    if(err != ESP_OK)
    {
        ret |= DIO_BANK0_WRITE_ERROR;
    }

    err = xl9555_set_output(dio_bank1_handle, 1, (uint8_t*)&value+1);
    if(err != ESP_OK)
    {
        ret |= DIO_BANK1_WRITE_ERROR;
    }
    return ret;
}

