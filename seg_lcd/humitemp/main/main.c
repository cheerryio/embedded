#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "humitemp_lcd.h"

#define PIN_NUM_WR 22
#define PIN_NUM_DATA 23
#define PIN_NUM_CS 21

spi_device_handle_t spi;
humitemp_lcd_handle_t humitemp_lcd_handle;

void initPeripherals(){
    esp_err_t ret;
    spi_bus_config_t buscfg={
        .mosi_io_num=PIN_NUM_DATA,
        .miso_io_num=-1,
        .sclk_io_num=PIN_NUM_WR,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=4*32+9
    };
    spi_device_interface_config_t devcfg={
        .mode=0,                                //SPI mode 0
        .clock_speed_hz=100*1000,           //Clock out at 100 kHz
        .spics_io_num=PIN_NUM_CS,               //CS pin
        .queue_size=7                           //We want to be able to queue 7 transactions at a time
    };
    //Initialize the SPI bus
    ret=spi_bus_initialize(HSPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);

    humitemp_lcd_create_handle(spi,&humitemp_lcd_handle);
    humitemp_lcd_init(humitemp_lcd_handle);
    humitemp_lcd_clear_all(humitemp_lcd_handle);
    humitemp_lcd_flush(humitemp_lcd_handle);
}

void app_main(void)
{
    initPeripherals();
    while(1){
        humitemp_lcd_clear_all(humitemp_lcd_handle);
        humitemp_lcd_flush(humitemp_lcd_handle);
        for(int pos=1;pos<=13;pos++){
            for(char alpha='a';alpha<='b';alpha++){
                humitemp_lcd_set_symbol(humitemp_lcd_handle,pos,alpha,0);
                humitemp_lcd_flush(humitemp_lcd_handle);
                vTaskDelay(10);
            }
            for(char digit='0';digit<='9';digit++){
                humitemp_lcd_set_symbol(humitemp_lcd_handle,pos,digit,pos==10?1:0);
                humitemp_lcd_flush(humitemp_lcd_handle);
                vTaskDelay(10);
            }
        }
        humitemp_lcd_set_s6(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_s7(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_s8(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_kpa(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_rh(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_set(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_com(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_alert(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_record(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_h1(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_l1(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_h2(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_l2(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_s1(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_s2(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_lx(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_c(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_ppm(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_s3(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_v(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_ma(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_mpa(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_state(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_s5(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_col(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_s4(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_t1(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_t5(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_t4(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_t3(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
        humitemp_lcd_set_t2(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(10);
    }
}