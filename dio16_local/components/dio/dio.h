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
#ifndef _IOT_FORCHINE_DIO16_DEVICE_H_
#define _IOT_FORCHINE_DIO16_DEVICE_H_

#define DIO_BANK0_READ_ERROR 1
#define DIO_BANK1_READ_ERROR 2
#define DIO_BANK0_WRITE_ERROR 4
#define DIO_BANK1_WRITE_ERROR 8
#define DIO_PIN_OUT_RANGE (0x10)
#define DIO_OK 0

#include <stdbool.h>
#include  "esp_err.h"
/**
 * @brief initialize the dio16 lowlevel module
 *
 * @param none
 *
 * @return none
 */
esp_err_t dio_device_init(void);

/**
 * @brief deinitialize the lowlevel module
 *
 * @param none
 *
 * @return none
 */
esp_err_t dio_device_deinit(void);


/**
 * @brief set the DO 
 *
 * @param pin The pin number (0~15)
 * @param value The pin value , 0 for off, 1 for on. 
 *
 * @return 
 *     - 0 : OK
 *     - others : fail
 */
int dio_set_dout(uint8_t pin, bool value);

/**
 * @brief get the DO state
 *
 * @param pin The pin number (0~15)
 * @param p_value pointer to the pin value , return value inside.. 
 *
 * @return 
 *     - 0 : OK
 *     - others : fail
 */
int dio_get_dout(uint8_t pin, bool *p_value);

/**
 * @brief get the DI state
 *
 * @param pin The pin number (0~15)
 * @param p_value pointer to the pin value , return value inside.. 
 *
 * @return 
 *     - 0 : OK
 *     - others : fail
 */
int dio_get_din(uint8_t pin, bool *p_value);

/**
 * @brief get the DI state all 16bit
 *
 * @param p_value pointer to the pin value , return value inside.. 
 *
 * @return 
 *     - 0 : OK
 *     - others : fail
 */
int dio_get_input16( uint16_t *p_value);

/**
 * @brief get the DO state all 16bit
 *
 * @param p_value pointer to the pin value , return value inside.. 
 *
 * @return 
 *     - 0 : OK
 *     - others : fail
 */
int dio_get_output16( uint16_t *p_value);

/**
 * @brief set the DO state all 16bit
 *
 * @param p_value the pin value , return value inside.. 
 *
 * @return 
 *     - 0 : OK
 *     - others : fail
 */
int dio_set_output16( uint16_t value);

#endif /* _IOT_FORCHINE_DIO16_DEVICE_H_ */
