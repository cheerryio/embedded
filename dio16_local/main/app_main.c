/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2019 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP32 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"

#include "infra_compat.h"


#include "driver/gpio.h"
#include "conn_mgr.h"

#include "iot_button.h"
#include "xl9555.h"
#include "board.h"

#include "mb_slave.h"
#include "modbus_params.h"
#include "esp_spiffs.h"
#include "cJSON.h"
#include "dio.h"

#include "nvs_flash.h"
#include "nvs.h"
#include "led_indicator.h"

#define TIMES 256
static const char *TAG = "DIO16";

#define DEVICE_OK         0x0000

#define BR_1200 0
#define BR_2400 1
#define BR_4800 2
#define BR_9600 3
#define BR_19200 4
#define BR_38400 5
#define BR_57600 6
#define BR_115200 7
int32_t bandrate_table[]={1200,2400,4800,9600,19200,38400,57600,115200};
int16_t mb_slave_addr = 8;
int32_t mb_baudrate = BR_9600;
uint8_t mb_parity =UART_PARITY_EVEN;
int32_t device_state = DEVICE_OK;
button_handle_t h_prov_key = NULL;

static led_indicator_handle_t h_485_red = NULL;
static led_indicator_handle_t h_485_green = NULL;
static led_indicator_handle_t h_net_red = NULL;
static led_indicator_handle_t h_net_green = NULL;

bool modbus_config_modified;
led_indicator_blink_type_t net_blink_red,net_blink_green;
char ota_reboot_flag=0;
void set_ota_reboot_flag()
{
    ota_reboot_flag=10;
}
#if(CONFIG_NVS_ENCRYPTION)
nvs_sec_cfg_t my_nvs_sec_cfg;
char nvs_secure_cfg_init()
{
    esp_partition_t *part = NULL;
    part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS_KEYS, NULL);
    if(part == NULL)
    {
        ESP_LOGE(TAG, "NVS_KEY_PART open failed.");
        return ESP_FAIL;
    }

    esp_err_t ret =nvs_flash_read_security_cfg(part, &my_nvs_sec_cfg);
    if(ret != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_flash_read_security_cfg failed.");
        return ESP_FAIL;
    }
    return 0;
}
#endif

int load_device_conf(void)
{
    int len = 16;
    nvs_handle_t my_handle;
    uint32_t hw_ver;
    esp_err_t err;
#if(CONFIG_NVS_ENCRYPTION)
	err = nvs_flash_secure_init_partition("fctry",&my_nvs_sec_cfg);
#else
    err = nvs_flash_init_partition("fctry");
#endif

    err = nvs_open_from_partition("fctry","device_conf_key", NVS_READONLY, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        uint32_t kv_temp;
        if(nvs_get_u32(my_handle, "Baudrate", &kv_temp) == ESP_OK)
        {
            mb_baudrate = kv_temp;
            ESP_LOGI(TAG, "kv load baudrate: %d" ,bandrate_table[mb_baudrate]);
        }
        
        if(nvs_get_u32(my_handle, "Parity", &kv_temp) == ESP_OK)
        {
            mb_parity = kv_temp;
            ESP_LOGI(TAG, "kv load parity %d" , mb_parity);
        }

        if(nvs_get_u32(my_handle, "SlaveAddr", &kv_temp) == ESP_OK)
        {
            mb_slave_addr = kv_temp;
            ESP_LOGI(TAG, "kv load slave address %d",mb_slave_addr);
        }
    }
    //todo: error handle.
    nvs_close(my_handle);
    nvs_flash_deinit_partition("fctry");

    return 0;
}

int save_device_conf(char* key,int value)
{
    nvs_handle_t my_handle;
    esp_err_t err;
#if(CONFIG_NVS_ENCRYPTION)
	err = nvs_flash_secure_init_partition("fctry",&my_nvs_sec_cfg);
#else
    err = nvs_flash_init_partition("fctry");
#endif

    err = nvs_open_from_partition("fctry","device_conf_key", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        if(nvs_set_u32(my_handle, key, (uint32_t)value) == ESP_OK)
        {
            ESP_LOGI(TAG, "kv save %s : %d OK." ,key,value);
        }
    }
    //todo: error handle.
    nvs_close(my_handle);
    nvs_flash_deinit_partition("fctry");

    return 0;
}

int save_modbus_conf()
{
    nvs_handle_t my_handle;
    esp_err_t err;
#if(CONFIG_NVS_ENCRYPTION)
	err = nvs_flash_secure_init_partition("fctry",&my_nvs_sec_cfg);
#else
    err = nvs_flash_init_partition("fctry");
#endif

    err = nvs_open_from_partition("fctry","device_conf_key", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        uint32_t value;char* key;
        value = mb_baudrate;
        key = "Baudrate";
        if(nvs_set_u32(my_handle, key, value) == ESP_OK)
        {
            ESP_LOGI(TAG, "kv save %s : %d OK." ,key,value);
        }

        value = mb_parity;
        key = "Parity";
        if(nvs_set_u32(my_handle, key, value) == ESP_OK)
        {
            ESP_LOGI(TAG, "kv save %s : %d OK." ,key,value);
        }

        value = mb_slave_addr;
        key = "SlaveAddr";
        if(nvs_set_u32(my_handle, key , value) == ESP_OK)
        {
            ESP_LOGI(TAG, "kv save %s : %d OK." ,key,value);
        }
    }
    //todo: error handle.
    nvs_close(my_handle);
    nvs_flash_deinit_partition("fctry");

    return 0;
}
/*
static esp_err_t wifi_event_handle(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case  SYSTEM_EVENT_STA_CONNECTED:
            ESP_LOGI(TAG, "wifi service event: CONNECTED.\n");
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
        case SYSTEM_EVENT_STA_LOST_IP:
//            link_yield_ms = 1000;
            //g_wifi_status = CODE_WIFI_ON_DISCONNECT;
            net_link_state = NET_WIFI_OFFLINE;
            ESP_LOGI(TAG, "wifi service event: DISCONNECT.\n");
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            net_link_state = NET_WIFI_CONNECTED;
            led_indicator_stop(h_net_green,net_blink_green);
            led_indicator_start(h_net_green, BLINK_WIFI_CONNECTED );
            net_blink_green = BLINK_WIFI_CONNECTED ;
            if (linkkit_started == false) {
                wifi_config_t wifi_config = {0};
                if (conn_mgr_get_wifi_config(&wifi_config) == ESP_OK &&
                    strcmp((char *)(wifi_config.sta.ssid), HOTSPOT_AP) &&
                    strcmp((char *)(wifi_config.sta.ssid), ROUTER_AP)) {
                    xTaskCreate((void (*)(void *))linkkit_main, "xl_dio16_link", 10240, NULL, 5, NULL);
                    linkkit_started = true;
                }
            }
            break;

        default:
            ESP_LOGI(TAG, "wifi service event: DISCONNECT.\n");
            break;
    }

    return ESP_OK;
}
*/
/* cloud propertys
temperature
humidity
mb_slave_addr
baudrate
parity
temp_alarm_type
humi_alarm_type
temp_control_enable bool
humi_control_enable   bool
temp_control_state
humi_control_state
device_state
temp_ceiling
temp_floor
humi_ceiling
humi_floor
*/
char* mb_set_report_payload = NULL;
void modbus_write_holdings_cb(uint16_t reg, uint8_t *data ,size_t size)
{
    if(reg == 0x20)
    {
        uint16_t vout = *(uint16_t*)data;
        uint16_t old ;
        dio_get_output16(&old);
        dio_set_output16(vout);
        coil_reg_params = *(coil_reg_params_t*)data;

        uint16_t chg_mask = old^vout;


        while(mb_set_report_payload ) {
                 vTaskDelay(pdMS_TO_TICKS(50));   //wait 50 ms.
        }
        if( (!mb_set_report_payload))
        {
            mb_set_report_payload = (char*)malloc(160);
            mb_set_report_payload[0] = 0;
            strcat (mb_set_report_payload ,"{ ");


            for(int i=0;i<16;i++)
            {       
                    if(chg_mask&BIT(i))
                    {
                        ESP_LOGI(TAG,"chg_mask %x",chg_mask);
                        char tItem[20]; int len=20;
                        snprintf(tItem,len,"\"out%d\":%d,",i,((vout&BIT(i)) !=0));
                        strcat(mb_set_report_payload,tItem); 
                    }
            }
//            mb_set_report_payload[strlen(mb_set_report_payload)-1] = '}';
        }
    }
}

void modbus_write_coils_cb(uint16_t pin_start, uint8_t *data, size_t size)
{
    uint8_t* p_coil_base8;
    uint16_t* p_coil_base16;
    if(pin_start>8)
    {
        p_coil_base8 = data-1;
    }
    else
    {
        p_coil_base8 = data;
    }
    p_coil_base16 = (uint16_t*)p_coil_base8;
    uint16_t dout16 = *p_coil_base16;

    for(int16_t i=0;i<size;i++)
    {
        bool v = (dout16 >> (pin_start+i))&0x0001;
        uint8_t pin = pin_start+i;
        dio_set_dout(pin,v);
        ESP_LOGI(TAG,"modbus_set_DOUT pin %d ,value  %x. ",pin_start+i, (dout16 >> (pin_start+i))&0x0001) ;
        uint16_t v_all =holding_reg_params.DO_A;
        if(v)
        {
            v_all |= BIT(pin);
        }
        else
        {
            v_all &= ~BIT(pin);
        }
        holding_reg_params.DO_A = v_all; 
    }


    while(mb_set_report_payload ) {
                 vTaskDelay(pdMS_TO_TICKS(50));   //wait 50 ms.
    }
    if((!mb_set_report_payload))
    {
        mb_set_report_payload = (char*)malloc(160);
        mb_set_report_payload[0] = 0;
        strcat (mb_set_report_payload ,"{ ");


        for(int16_t i=0;i<size;i++)
        {       
            char tItem[20]; int len=20;
            bool v = (dout16 >> ((pin_start+i))&0x0001) !=0;
            uint8_t pin = pin_start+i;
            snprintf(tItem,len,"\"out%d\":%d,",pin,v);   
            strcat(mb_set_report_payload,tItem);            
        }
//        mb_set_report_payload[strlen(mb_set_report_payload)-1] = '}';
    }
    return;
}

static void dio16mon_task(void *p)
{
    while(1)
    {
        uint16_t in_pin16;
        dio_get_input16(&in_pin16);

        uint16_t chg_mask = in_pin16^ holding_reg_params.DI_A;
        if(chg_mask)
        {
            holding_reg_params.DI_A = in_pin16;
            discrete_reg_params = *(discrete_reg_params_t *)&in_pin16;
            input_reg_params = *(input_reg_params_t *)&in_pin16;
            

            if(mb_set_report_payload ) {
                    continue;   //no link report.
            }
            if((!mb_set_report_payload))
            {
                mb_set_report_payload = (char*)malloc(160);
                mb_set_report_payload[0] = 0;
                strcat (mb_set_report_payload ,"{ ");


                for(int i=0;i<16;i++)
                {       
                        if(chg_mask&BIT(i))
                        {
                            ESP_LOGI(TAG,"chg_mask %x",chg_mask);
                            char tItem[20]; int len=20;
                            snprintf(tItem,len,"\"in%d\":%d,",i,((in_pin16&BIT(i)) !=0));
                            strcat(mb_set_report_payload,tItem); 
                        }
                }
                mb_set_report_payload[strlen(mb_set_report_payload)-1] = '}';
            }
        }
        vTaskDelay(pdMS_TO_TICKS(30));
    }      
}

void spiffs_init()
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = false
    };

    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    /* The following calls demonstrate reading files from the generated SPIFFS
     * image. The images should contain the same files and contents as the spiffs_image directory.
     */

    // Read and display the contents of a small text file (hello.txt)
//    read_hello_txt();

    // All done, unmount partition and disable SPIFFS
    //esp_vfs_spiffs_unregister(NULL);
    //ESP_LOGI(TAG, "SPIFFS unmounted");
}

esp_err_t conn_mgr_is_configured(bool *configured);


static void func_key_click_cb(void *arg)
{
    ESP_LOGI(TAG, "key pressed");

        
    led_indicator_stop(h_net_green,net_blink_green);
    led_indicator_start(h_net_green, BLINK_AWSS_PRESS );
    net_blink_green = BLINK_AWSS_PRESS ;


    if(modbus_config_modified)
    {
        save_modbus_conf();
        vTaskDelay(pdMS_TO_TICKS(200));
        esp_restart();
    }
}

static void func_key_longpress_cb(void *arg)
{

    led_indicator_start(h_net_green, BLINK_FACTORY_RESET );
    
    vTaskDelay(pdMS_TO_TICKS(2000));

}



void app_main()
{
//    spiffs_init();
#if(CONFIG_NVS_ENCRYPTION)
    nvs_secure_cfg_init();
#endif
    load_device_conf();

    iot_board_init();
    dio_device_init();

    h_prov_key = (button_handle_t)iot_board_get_handle(BOARD_BUTTON_ID);
    iot_button_register_cb(h_prov_key, BUTTON_SINGLE_CLICK, func_key_click_cb);
    iot_button_register_cb(h_prov_key, BUTTON_LONG_PRESS_START, func_key_longpress_cb);
    IOT_SetLogLevel(IOT_LOG_INFO);

    h_485_red = (led_indicator_handle_t)iot_board_get_handle(BOARD_LED_485_R);
    h_485_green = (led_indicator_handle_t)iot_board_get_handle(BOARD_LED_485_G);
    h_net_red = (led_indicator_handle_t)iot_board_get_handle(BOARD_LED_NET_R);
    h_net_green = (led_indicator_handle_t)iot_board_get_handle(BOARD_LED_NET_G);



    xTaskCreate(dio16mon_task, "dio16mon_task", 4096+4096, NULL, 5, NULL);
    xTaskCreate((void (*)(void *))mb_slave_task_func,"modbus_task", 4096+2048,NULL, 5,NULL);

    led_indicator_start(h_net_green, BLINK_WIFI_NOCONFIG );
    net_blink_green = BLINK_WIFI_NOCONFIG ;

}




void App_Config_Persistence_Start(void)
{

    ESP_LOGI(TAG, "Config Persistence Start.\n");

}
void App_Config_Persistence_Write(char *buffer, uint32_t length)
{
    ESP_LOGI(TAG, "Config Persistence Write length  %d \n",length);


}
void App_Config_Persistence_Stop(void)
{
    ESP_LOGI(TAG, "Config Persistence Stop.\n");
}

/*
static const unsigned char wifi24_16[]={
0x00, 0x20, 0x30, 0x18, 0x8c, 0xc4, 0x44, 0x44, 0x44, 0xc4, 0x8c, 0x18, 0x30, 0x20, 0x00, 0x00,
0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x08, 0x1c, 0x08, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const unsigned char wifix24_16[]={
0x00, 0x20, 0x30, 0x18, 0x8c, 0xc4, 0x44, 0x44, 0x44, 0xc4, 0x8c, 0x18, 0x30, 0x20, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x08, 0x1c,
0x08, 0x00, 0x01, 0x01, 0x00, 0x00, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const unsigned char wificloud_24_16[]={
0x00, 0x20, 0x30, 0x18, 0x8c, 0xc4, 0x44, 0x44, 0x44, 0xc4, 0x8c, 0x18, 0x30, 0x20, 0x00, 0x00,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x08, 0x1c,
0x08, 0x00, 0x01, 0x01, 0x00, 0x00, 0x0a, 0x1b, 0x3b, 0x1b, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const unsigned char modbus16[]={
0x00, 0x06, 0x05, 0x9f, 0xc4, 0x80, 0x0a, 0x15, 0x15, 0x0a, 0x00, 0xd7, 0xd5, 0xcd, 0x00, 0x00,
0xc0, 0xc2, 0xc3, 0xff, 0xff, 0xff, 0xc3, 0xc2, 0xc0, 0xc4, 0xcc, 0xdf, 0xff, 0xdf, 0xcc, 0xc4
};
*/
/*
*/

/*
static const unsigned char bmp_image_52_24[156] = { // 0X02,0X01,0X34,0X00,0X18,0X00, 
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X1E, 0X82, 0X86, 0X02, 0X1E, 0X06, 0X38, 0X1E, 0X80, 0X06,
    0X1B, 0X1B, 0X00, 0X1A, 0X1C, 0X00, 0X9C, 0X80, 0X80, 0X98, 0X98, 0X80, 0X98, 0X80, 0X83, 0X81,
    0X80, 0X80, 0X00, 0X20, 0X08, 0X00, 0X00, 0X40, 0XC0, 0X04, 0X00, 0X04, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X01, 0X80, 0XC0, 0X41, 0X60, 0X60,
    0X20, 0X70, 0X31, 0X18, 0XAC, 0X24, 0X3E, 0X3E, 0X27, 0X31, 0X31, 0X25, 0X35, 0X34, 0X34, 0X20,
    0XF0, 0XBF, 0X3C, 0X39, 0X25, 0X32, 0X39, 0XBD, 0X1A, 0X42, 0X04, 0X2C, 0X38, 0X30, 0X20, 0XC0,
    0X80, 0X02, 0X06, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X3C, 0X3F, 0X87,
    0XF8, 0XFC, 0XFE, 0XA6, 0X87, 0XDE, 0XFE, 0XFC, 0XE0, 0XC1, 0XDE, 0XC0, 0XE0, 0XE0, 0XE0, 0XE0,
    0XE0, 0XE0, 0XA0, 0XA0, 0XA0, 0XA0, 0XC0, 0XC0, 0XC0, 0XDE, 0XC9, 0XE2, 0XFF, 0XFF, 0XDE, 0X87,
    0X87, 0XFE, 0XFE, 0XFC, 0XAD, 0XBF, 0X1E, 0X00, 0X00, 0X00, 0X00, 0X00
};*/


