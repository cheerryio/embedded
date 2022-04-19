// Copyright 2021-2022 Wuhan Forchine Tech. LTD
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
#ifndef _IOT_FORCHINE_XL9555_H_
#define _IOT_FORCHINE_XL9555_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "esp_err.h"
#include "i2c_bus.h"

#define XL9555_ADDR_PREFIX 0x20 // combine with A2 A1 A0 value.
typedef void* xl9555_handle_t;

/**
 * @brief Create xl9555_handle_t
 *
 * @param i2c bus controller object handle
 * @param dev_addr xl9555 i2c slave address
 *
 * @return
 *     - sht3x handle_t
 */
xl9555_handle_t xl9555_create(i2c_bus_handle_t bus, uint8_t dev_addr);


/**
 * @brief Delete xl9555_handle_t
 *
 * @param pointer to object handle of xl9555
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t xl9555_delete(xl9555_handle_t *sensor);



/**
 * @brief set port direction 
 *
 * @param object handle of xl9555
 * @param port 0,1
 * @param direction mask, 0 for output, 1 for input(hardware power on default).	
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t xl9555_set_dir(xl9555_handle_t  dev_handle , uint8_t port, uint8_t dirmask);


/**
 * @brief set input inversion	
 *
 * @param object handle of xl9555
 * @param port 0,1
 * @param inversion mask, 1 for invert, hardward default 0.	
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t xl9555_set_inv(xl9555_handle_t  dev_handle , uint8_t port, uint8_t invmask);


/**
 * @brief Delete xl9555_handle_t
 *
 * @param object handle of xl9555
 * @param port 0,1
 * @param value, 8 bit input value for a port.	
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t xl9555_get_input(xl9555_handle_t  dev_handle, uint8_t port, uint8_t* p_value);

/**
 * @brief Delete xl9555_handle_t
 *
 * @param object handle of xl9555
 * @param port 0,1
 * @param value, 8 bit  value for a outport.	
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t xl9555_get_output(xl9555_handle_t dev_handle, uint8_t port, uint8_t* p_value);

/**
 * @brief Delete xl9555_handle_t
 *
 * @param object handle of xl9555
 * @param port 0,1
 * @param value, 8 bit output value for a port.	
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t xl9555_set_output(xl9555_handle_t  dev_handle, uint8_t port, uint8_t* p_value);


#ifdef __cplusplus
}
#endif

#endif /* _IOT_FORCHINE_XL9555_H_ */
