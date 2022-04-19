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

#include "humitemp_lcd.h"

static const char *TAG = "test_humitemp_lcd";

#define DELAY 100

spi_device_handle_t spi;
humitemp_lcd_handle_t humitemp_lcd_handle;

static void humitemp_lcd_setup(){
    spi_setup(&spi);

    humitemp_lcd_create_handle(spi,&humitemp_lcd_handle);
    humitemp_lcd_init(humitemp_lcd_handle);
    humitemp_lcd_clear_all(humitemp_lcd_handle);
    humitemp_lcd_flush(humitemp_lcd_handle);
}

static void humitemp_lcd_teardown(){
    humitemp_lcd_delete_handle(humitemp_lcd_handle);
}

TEST_CASE("test","[humitemp lcd]")
{
    printf("hello\n");
}

TEST_CASE("light up","[humitemp lcd]")
{
    humitemp_lcd_setup();
    humitemp_lcd_clear_all(humitemp_lcd_handle);
    humitemp_lcd_flush(humitemp_lcd_handle);
    for(int pos=1;pos<=13;pos++){
        for(char digit='0';digit<='9';digit++){
            humitemp_lcd_set_symbol(humitemp_lcd_handle,pos,digit,pos==10?1:0);
            humitemp_lcd_flush(humitemp_lcd_handle);
            vTaskDelay(DELAY);
        }
    }
    humitemp_lcd_set_s6(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_s7(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_s8(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_kpa(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_rh(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_set(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_com(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_alert(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_record(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_h1(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_l1(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_h2(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_l2(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_s1(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_s2(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_lx(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_c(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_ppm(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_s3(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_v(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_ma(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_mpa(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_state(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_s5(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_col(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_s4(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_t1(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_t5(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_t4(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_t3(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_set_t2(humitemp_lcd_handle,1);humitemp_lcd_flush(humitemp_lcd_handle);vTaskDelay(DELAY);
    humitemp_lcd_teardown();
}