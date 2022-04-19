#ifndef __HUMITEMP_LCD_H__
#define __HUMITEMP_LCD_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#define PIN_NUM_WR 22
#define PIN_NUM_DATA 23
#define PIN_NUM_CS 21

#define SYS_DIS ((0b0000<<4) | 0b0000)
#define SYS_EN ((0b0000<<4) | 0b0001)
#define LCD_OFF ((0b0000<<4) | 0b0010)
#define LCD_ON ((0b0000<<4) | 0b0011)
#define TIMER_DIS ((0b0000<<4) | 0b0100)
#define WDT_DIS ((0b0000<<4) | 0b0101)
#define TIMER_EN ((0b0000<<4) | 0b0110)
#define WDT_EN ((0b0000<<4) | 0b0111)
#define TONE_OFF ((0b0000<<4) | 0b1000)
#define TONE_ON ((0b0000<<4) | 0b1001)
#define BIAS4_1_3 (0x29)
#define RC256 ((0b0001<<4) | 0b1000)

typedef struct {
    spi_device_handle_t spi;
    uint8_t mem[32];
} humitemp_lcd_t;
typedef humitemp_lcd_t* humitemp_lcd_handle_t;

esp_err_t spi_setup(spi_device_handle_t *handle);
esp_err_t humitemp_lcd_create_handle(spi_device_handle_t spi,humitemp_lcd_handle_t* handle_ptr);
esp_err_t humitemp_lcd_delete_handle(humitemp_lcd_handle_t handle);
esp_err_t humitemp_lcd_init(humitemp_lcd_handle_t handle);
esp_err_t humitemp_lcd_set_symbol(humitemp_lcd_handle_t handle,int position, char symbol,int dp);
esp_err_t humitemp_lcd_flush(humitemp_lcd_handle_t handle);
esp_err_t humitemp_lcd_clear_all(humitemp_lcd_handle_t handle);
void humitemp_lcd_set_s6(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_s7(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_s8(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_kpa(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_rh(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_set(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_com(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_alert(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_record(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_h1(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_l1(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_h2(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_l2(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_s1(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_s2(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_lx(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_c(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_ppm(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_s3(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_v(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_ma(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_mpa(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_state(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_s5(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_col(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_s4(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_t1(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_t5(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_t4(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_t3(humitemp_lcd_handle_t handle,int on);
void humitemp_lcd_set_t2(humitemp_lcd_handle_t handle,int on);

#endif