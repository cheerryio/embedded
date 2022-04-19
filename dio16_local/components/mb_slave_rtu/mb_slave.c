/*

 * Copyright (c) 2022 <WUHAN FORCHINE TECHNOLOGY LTD>
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include <stdio.h>
#include <math.h>
#include <sdkconfig.h>
#include "mbcontroller.h"       // for mbcontroller defines and api
#include "modbus_params.h"  
#include "esp_log.h"
#include "board.h"
#include "led_indicator.h"


#define MB_PORT_NUM     2
extern int32_t bandrate_table[];
extern int16_t mb_slave_addr ;
extern int32_t mb_baudrate ;
extern uint8_t mb_parity;


#define MB_PAR_INFO_GET_TOUT                (10) // Timeout for get parameter info
#define MB_CHAN_DATA_MAX_VAL                (6)
#define MB_CHAN_DATA_OFFSET                 (0.2f)

#define MB_READ_MASK                        (MB_EVENT_INPUT_REG_RD \
                                                | MB_EVENT_HOLDING_REG_RD \
                                                | MB_EVENT_DISCRETE_RD \
                                                | MB_EVENT_COILS_RD)
#define MB_WRITE_MASK                       (MB_EVENT_HOLDING_REG_WR \
                                                | MB_EVENT_COILS_WR)
#define MB_READ_WRITE_MASK                  (MB_READ_MASK | MB_WRITE_MASK)
static const char *SLAVE_TAG = "MB_SLAVE";

static portMUX_TYPE param_lock = portMUX_INITIALIZER_UNLOCKED;



// Here are the user defined instances for device parameters packed by 1 byte
// These are keep the values that can be accessed from Modbus master
holding_reg_params_t holding_reg_params = { 0 ,0,0,0};

input_reg_params_t input_reg_params = { 0 };

coil_reg_params_t coil_reg_params = { 0 };

discrete_reg_params_t discrete_reg_params = { 0 };


void modbus_write_holdings_cb(uint16_t reg, uint8_t *data ,size_t size);

void modbus_write_coils_cb(uint16_t pin_start, uint8_t *data, size_t size);


static led_indicator_handle_t    h_mb_red ;
static led_indicator_handle_t    h_mb_green ;

void mb_slave_task_func(void* p)
{
    mb_param_info_t reg_info; // keeps the Modbus registers access information
    mb_communication_info_t comm_info; // Modbus communication parameters
    mb_register_area_descriptor_t reg_area; // Modbus register area descriptor structure

    // Set UART log level
    esp_log_level_set(SLAVE_TAG, ESP_LOG_INFO);
    void* mbc_slave_handler = NULL;
    h_mb_red = (led_indicator_handle_t)iot_board_get_handle(BOARD_LED_485_R);
    h_mb_green = (led_indicator_handle_t)iot_board_get_handle(BOARD_LED_485_G);

    ESP_ERROR_CHECK(mbc_slave_init(MB_PORT_SERIAL_SLAVE, &mbc_slave_handler)); // Initialization of Modbus controller

    comm_info.mode = MB_MODE_RTU,
    comm_info.slave_addr = mb_slave_addr;
    comm_info.port = MB_PORT_NUM;
    comm_info.baudrate = bandrate_table[mb_baudrate];
    comm_info.parity = mb_parity;
    ESP_ERROR_CHECK(mbc_slave_setup((void*)&comm_info));

    // The code below initializes Modbus register area descriptors
    // for Modbus Holding Registers, Input Registers, Coils and Discrete Inputs
    // Initialization should be done for each supported Modbus register area according to register map.
    // When external master trying to access the register in the area that is not initialized
    // by mbc_slave_set_descriptor() API call then Modbus stack
    // will send exception response for this register area.
    reg_area.type = MB_PARAM_HOLDING; // Set type of register area
    reg_area.start_offset = MB_REG_HOLDING_START; // Offset of register area in Modbus protocol
    reg_area.address = (void*)&holding_reg_params; // Set pointer to storage instance
    reg_area.size = sizeof(holding_reg_params); // Set the size of register storage instance
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

    // Initialization of Input Registers area
    reg_area.type = MB_PARAM_INPUT;
    reg_area.start_offset = MB_REG_INPUT_START;
    reg_area.address = (void*)&input_reg_params;
    reg_area.size = sizeof(input_reg_params);
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

    // Initialization of Coils register area
    reg_area.type = MB_PARAM_COIL;
    reg_area.start_offset = MB_REG_COILS_START;
    reg_area.address = (void*)&coil_reg_params;
    reg_area.size = sizeof(coil_reg_params);
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

    // Initialization of Discrete Inputs register area
    reg_area.type = MB_PARAM_DISCRETE;
    reg_area.start_offset = MB_REG_DISCRETE_INPUT_START;
    reg_area.address = (void*)&discrete_reg_params;
    reg_area.size = sizeof(discrete_reg_params);
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

//    setup_reg_data(); // Set values into known state

    // Starts of modbus controller and stack
    ESP_ERROR_CHECK(mbc_slave_start());

    // Set UART pin numbers
    ESP_ERROR_CHECK(uart_set_pin(MB_PORT_NUM, BOARD_IO_UART2_TXD,
                            BOARD_IO_UART2_RXD, BOARD_IO_485_CON,
                            UART_PIN_NO_CHANGE));

    // Set UART driver mode to Half Duplex
    ESP_ERROR_CHECK(uart_set_mode(MB_PORT_NUM, UART_MODE_RS485_HALF_DUPLEX));

    ESP_LOGI(SLAVE_TAG, "Modbus slave stack initialized.");
    ESP_LOGI(SLAVE_TAG, "Start modbus slave...");

    // The cycle below will be terminated when parameter holdingRegParams.dataChan0
    // incremented each access cycle reaches the CHAN_DATA_MAX_VAL value.
    //for(;holding_reg_params.holding_data0 < MB_CHAN_DATA_MAX_VAL;) {
    while(1){
        // Check for read/write events of Modbus master for certain events
        mb_event_group_t event = mbc_slave_check_event(MB_READ_WRITE_MASK);
        ESP_LOGI(SLAVE_TAG,"mb event check.");
        const char* rw_str = (event & MB_READ_MASK) ? "READ" : "WRITE";
        led_indicator_start(h_mb_green, BLINK_MODBUS_COMM );
        // Filter events and process them accordingly
        if(event & (MB_EVENT_HOLDING_REG_WR | MB_EVENT_HOLDING_REG_RD)) {           
            // Get parameter information from parameter queue
            led_indicator_start(h_mb_green, BLINK_MODBUS_COMM );

            ESP_ERROR_CHECK(mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
            ESP_LOGI(SLAVE_TAG, "HOLDING %s (%u us), ADDR:%u, TYPE:%u, INST_ADDR:0x%.4x, SIZE:%u",
                    rw_str,
                    (uint32_t)reg_info.time_stamp,
                    (uint32_t)reg_info.mb_offset,
                    (uint32_t)reg_info.type,
                    (uint32_t)reg_info.address,
                    (uint32_t)reg_info.size);
            if(reg_info.mb_offset == 0x20)
            {
                modbus_write_holdings_cb(reg_info.mb_offset, reg_info.address ,reg_info.size);
            }
            /* democode, to be review.
            if (reg_info.address == (uint8_t*)&holding_reg_params.holding_data)
            {
                portENTER_CRITICAL(&param_lock);
                //holding_reg_params.holding_data[0] += MB_CHAN_DATA_OFFSET;
               // if (holding_reg_params.holding_data >= (MB_CHAN_DATA_MAX_VAL - MB_CHAN_DATA_OFFSET)) {
                    //coil_reg_params.coils_port1 = 0xFF;
                //}
                portEXIT_CRITICAL(&param_lock);
            }*/
        } else if (event & MB_EVENT_INPUT_REG_RD) {
            ESP_ERROR_CHECK(mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
            ESP_LOGI(SLAVE_TAG, "INPUT READ (%u us), ADDR:%u, TYPE:%u, INST_ADDR:0x%.4x, SIZE:%u",
                    (uint32_t)reg_info.time_stamp,
                    (uint32_t)reg_info.mb_offset,
                    (uint32_t)reg_info.type,
                    (uint32_t)reg_info.address,
                    (uint32_t)reg_info.size);
        } else if (event & MB_EVENT_DISCRETE_RD) {
            ESP_ERROR_CHECK(mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
            ESP_LOGI(SLAVE_TAG, "DISCRETE READ (%u us): ADDR:%u, TYPE:%u, INST_ADDR:0x%.4x, SIZE:%u",
                                (uint32_t)reg_info.time_stamp,
                                (uint32_t)reg_info.mb_offset,
                                (uint32_t)reg_info.type,
                                (uint32_t)reg_info.address,
                                (uint32_t)reg_info.size);
        } else if (event & MB_EVENT_COILS_RD ) {
            ESP_ERROR_CHECK(mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
            ESP_LOGI(SLAVE_TAG, "COILS %s (%u us), ADDR:%u, TYPE:%u, INST_ADDR:0x%.4x, SIZE:%u",
                                rw_str,
                                (uint32_t)reg_info.time_stamp,
                                (uint32_t)reg_info.mb_offset,
                                (uint32_t)reg_info.type,
                                (uint32_t)reg_info.address,
                                (uint32_t)reg_info.size);
            //if (coil_reg_params.coils_port1 == 0xFF) break; democode, to be review.
        }
        else if (event &  MB_EVENT_COILS_WR) {
            ESP_ERROR_CHECK(mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
            ESP_LOGI(SLAVE_TAG, "COILS %s (%u us), ADDR:%u, TYPE:%u, INST_ADDR:0x%.4x, SIZE:%u",
                                rw_str,
                                (uint32_t)reg_info.time_stamp,
                                (uint32_t)reg_info.mb_offset,
                                (uint32_t)reg_info.type,
                                (uint32_t)reg_info.address,
                                (uint32_t)reg_info.size);
            modbus_write_coils_cb(reg_info.mb_offset - MB_REG_COILS_START,
                               reg_info.address,
                               reg_info.size);
            ESP_LOGI(SLAVE_TAG, "INST_ADDR value :%x", *(uint16_t*)(reg_info.address));
            //if (coil_reg_params.coils_port1 == 0xFF) break; democode, to be review.
        }
    }
    // Destroy of Modbus controller on alarm
    ESP_LOGI(SLAVE_TAG,"Modbus controller destroyed.");
    vTaskDelay(100);
    ESP_ERROR_CHECK(mbc_slave_destroy());
}