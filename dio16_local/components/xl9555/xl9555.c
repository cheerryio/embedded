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
#include "xl9555.h"
#include "esp_log.h"

static const char *TAG = "XL9555";

#define INPUT_P0  0
#define INPUT_P1  1
#define OUTPUT_P0 2
#define OUTPUT_P1 3
#define INV_P0    4
#define INV_P1    5
#define DIR_P0    6
#define DIR_P1    7


typedef struct {
    i2c_bus_device_handle_t i2c_dev;
    uint8_t dev_addr;
} xl9555_dev_t;


xl9555_handle_t xl9555_create(i2c_bus_handle_t bus, uint8_t dev_addr)
{
    xl9555_dev_t *dio_chip = (xl9555_dev_t *) calloc(1, sizeof(xl9555_dev_t));
    dio_chip->i2c_dev = i2c_bus_device_create(bus, dev_addr, i2c_bus_get_current_clk_speed(bus));
    if (dio_chip->i2c_dev == NULL) {
        free(dio_chip);
        return NULL;
    }
    dio_chip->dev_addr = dev_addr;
    return (xl9555_handle_t) dio_chip;
}


esp_err_t xl9555_delete(xl9555_handle_t* dev_handle)
{
    if (*dev_handle == NULL) {
        return ESP_OK;
    }

    i2c_bus_device_delete(&(((xl9555_dev_t *)(*dev_handle))->i2c_dev));
    free(*dev_handle);
    *dev_handle = NULL;
    return ESP_OK;
}


esp_err_t xl9555_set_dir(xl9555_handle_t dev_handle , uint8_t port, uint8_t dirmask)
{
    xl9555_dev_t *device = (xl9555_dev_t *) dev_handle;
    esp_err_t ret = i2c_bus_write_byte(device->i2c_dev, DIR_P0+port, dirmask);
    return ret;
}

esp_err_t xl9555_set_inv(xl9555_handle_t dev_handle , uint8_t port, uint8_t invmask)
{
    xl9555_dev_t *device = (xl9555_dev_t *) dev_handle;
    esp_err_t ret = i2c_bus_write_byte(device->i2c_dev, INV_P0+port, invmask);
    return ret;
}


esp_err_t xl9555_get_input(xl9555_handle_t dev_handle, uint8_t port, uint8_t* p_value)
{
    xl9555_dev_t *device = (xl9555_dev_t *) dev_handle;
    esp_err_t ret = i2c_bus_read_byte(device->i2c_dev, INPUT_P0+port, p_value);
    return ret;
}


esp_err_t xl9555_set_output(xl9555_handle_t dev_handle, uint8_t port, uint8_t* p_value)
{
    xl9555_dev_t *device = (xl9555_dev_t *) dev_handle;
    esp_err_t ret = i2c_bus_write_byte(device->i2c_dev, OUTPUT_P0+port, *p_value);
    return ret;
}

esp_err_t xl9555_get_output(xl9555_handle_t dev_handle, uint8_t port, uint8_t* p_value)
{
    xl9555_dev_t *device = (xl9555_dev_t *) dev_handle;
    esp_err_t ret = i2c_bus_read_byte(device->i2c_dev, OUTPUT_P0+port, p_value);
    return ret;
}
