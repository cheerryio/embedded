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
#ifndef _IOT_BOARD_H_
#define _IOT_BOARD_H_

#include "esp_err.h"
#include "i2c_bus.h"
#include "xl9555.h"
//#include "spi_bus.h"

/*ENABLE Initialization Process in iot_board_init(void)*/
#define _ENABLE 1
#define _DISABLE 0
#define _UNDEFINE

/**
 * Resource ID on Board,
 * You can use iot_board_get_handle(ID,handle) to refrence Resource's Handle
 * */
typedef enum {
    NULL_RESOURCE,
    BOARD_XL9555_BANK0_ID,
    BOARD_XL9555_BANK1_ID,
    BOARD_BUTTON_ID,
    BOARD_LED_485_R,
    BOARD_LED_485_G,
    BOARD_LED_NET_R,
    BOARD_LED_NET_G
}board_res_id_t;

typedef void* board_res_handle_t;

/*Definations of Board*/
#define BOARD_NAME "XL DIO16 Ver.A1"
#define BOARD_VENDOR "Wuhan Xianlong, firmware by Forchine Ltd."
#define BOARD_URL "https://devops.aliyun.com/XXXX"

/*Definations of MCU*/
#define BOARD_MCU_NAME "ESP32"
#define BOARD_MCU_FLASH_SIZE "4MB"
#define BOARD_MCU_RAM_SIZE "520KB"
#define BOARD_MCU_RAM_USER_SIZE "320KB"

/*Definations of IO*/
#define BOARD_IO_BUTTON_A 32
#define BOARD_IO_BUTTON_B _UNDEFINE
#define BOARD_IO_BUTTON_C _UNDEFINE

#define BOARD_IO_I2C2_SCL 18
#define BOARD_IO_I2C2_SDA 19

#define BOARD_IO_UART2_TXD 16
#define BOARD_IO_UART2_RXD 17
#define BOARD_IO_485_CON 5

#define BOARD_IO_LED_485_R 21
#define BOARD_IO_LED_485_G 22
#define BOARD_IO_LED_NET_R 23
#define BOARD_IO_LED_NET_G 25

#define BOARD_I2C_ADDR_XL9555_BANK0 (XL9555_ADDR_PREFIX | 0x06) 
#define BOARD_I2C_ADDR_XL9555_BANK1 (XL9555_ADDR_PREFIX | 0x07)
//0x06)
//0x03)
#define XL9555_BANK0_P0_DIR 0xFF //All input
#define XL9555_BANK0_P1_DIR 0x00 //All output
#define XL9555_BANK1_P0_DIR 0xFF
#define XL9555_BANK1_P1_DIR 0x00
/*Definations of Peripheral*/
#define BOARD_I2C_MODE I2C_MODE_MASTER
#define BOARD_I2C_SPEED (100000)
#define BOARD_I2C_SCL_PULLUP_EN _ENABLE
#define BOARD_I2C_SDA_PULLUP_EN _ENABLE


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Board level init.
 *        Peripherals can be chosen through menuconfig, which will be initialized with default configurations during iot_board_init.
 *        After board init, initialized peripherals can be referenced by handles directly.
 * 
 * @return esp_err_t 
 */
esp_err_t iot_board_init(void);

/**
 * @brief Board level deinit.
 *        After board deinit, initialized peripherals will be deinit and related handles will be set to NULL.
 * 
 * @return esp_err_t 
 */
esp_err_t iot_board_deinit(void);

/**
 * @brief Check if board is initialized 
 * 
 * @return true if board is initialized
 * @return false if board is not initialized
 */
bool iot_board_is_init(void);

/**
 * @brief Using resource's ID declared in board_res_id_t to get board level resource's handle
 * 
 * @param id Resource's ID declared in board_res_id_t
 * @return board_res_handle_t Resource's handle
 * if no related handle,NULL will be returned
 */
board_res_handle_t iot_board_get_handle(board_res_id_t id);

/**
 * @brief Get board information
 * 
 * @return String include BOARD_NAME etc. 
 */
char* iot_board_get_info();

#ifdef __cplusplus
}
#endif

#endif /* _IOT_BOARD_H_ */
