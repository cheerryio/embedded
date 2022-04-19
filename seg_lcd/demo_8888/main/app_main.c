/*
 *
 * Copyright (c) 2022 <FORCHINE TECHNOLOGY (WUHAN) LTD>
 *
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
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "spi_bus.h"

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define TRUE 1
#define FALSE 0
/*  HT1621B在esp32上的点亮验证。
本代码的目的： 
1.点亮1621B段码屏
2.评估硬件接口技术特性
3.评估软件显示接口方式

实现情况：
采用硬件：ESP32_NodeMCU， 4位数段码屏（相关技术资料详见doc文件夹）
硬件连线：      SPI_SCK_IO 23
            SPI_MOSI_IO 22
            SPI_MISO_IO 19 （未接，esp32 spi组件占用）
            HT1621B_CS_PIN 21
            
底层软件接口 esp-iot-solution 的spi_bus组件。
spi_bus组件提供 8位 16位 32位的spi写函数。
HT1621有着 3bit命令头 + 6bit地址码/或命令代码 的9bit头形态。
命令字可以利用16bit传输前9比特值，后面填充7bit0无妨。但是每个命令传完需要拉高CS，否则处于连续命令模式应需要拼接尾部。
初始化过程无视效率，按照上述方式处理。写入显示内容时，考虑效率采取连续写入拼装好的缓冲区方式。
注意spi组件的速率较高，要配置到HT1621B支持的范围以内，WR最大300KHz，典型150KHz.

软件驱动： 命令拼装成16比特发送，保证前9比特符合定义。
          显示采用一个64比特的long long缓冲字disp_buff_bundle，包含前面的写数据命令头101。采用long long的原因是易于整体移位。
          整个显示缓冲字的bit映像如下： MSB在前，总共有效数据是5字节+1bit。显示数字时，先把对应的seg_map字节移动到临时long long的
          最前面，然后后移9bit加上位置*8bit。更新显示缓冲字。最后用连续3个16bit写，把显示缓冲字刷到屏上。
          101    0 0000 0              XXXXXXX  XXXXXXXX ... 
          3bit   6bit start addr       32bit显示映像。
          所以初始值是 0xA000 0000 0000 0000
          
本试验软件在4位屏上形成的外部接口：
void show_digital_4(uint8_t d, uint8_t pos , uint8_t show) ，支持数字、3个小数点、1个冒号。
          
屏驱动程序的外部呈现接口形态建议：
lcd_display_icon(icon_id, show_flag);
lcd_display_char(value, position, show_flag); //小数点放在这里也可，因为它跟数字/字符绑定密切。 position是在数字/字符 在屏上的编序。
          
*/
#define SPI_SCK_IO 22
#define SPI_MOSI_IO 23
#define SPI_MISO_IO 25

#define HT1621B_CS_PIN 21

#define BIAS (0x29<<1)
#define RC256 0x30
#define  WDTDIS1 0x0A
#define TIMERDIS 0x08
#define SYSEN 0x02
#define LCDON 0x06
#define SYSDIS 0x00

#define SEG_POINT 10
void spi_bus_init_deinit_test()
{

}

/* connect mosi with miso for transfer test */
spi_bus_device_handle_t device_handle = NULL;

spi_bus_handle_t bus_handle = NULL;
void spi_bus_transfer_test()
{

    spi_config_t bus_conf = {
        .miso_io_num = SPI_MISO_IO,
        .mosi_io_num = SPI_MOSI_IO,
        .sclk_io_num = SPI_SCK_IO,
    };

    spi_device_config_t device_conf = {
        .cs_io_num = NULL_SPI_CS_PIN,
        .mode = 0	,
        .clock_speed_hz = 100 * 1000,
    };

    bus_handle = spi_bus_create(SPI2_HOST, &bus_conf);

    device_handle = spi_bus_device_create(bus_handle, &device_conf);

    printf("************reg16 transfer test***************\n");

    uint16_t in = 0;
    gpio_set_level(HT1621B_CS_PIN, 0);
    vTaskDelay(2);
    gpio_set_level(HT1621B_CS_PIN, 1);
    vTaskDelay(2);
    gpio_set_level(HT1621B_CS_PIN, 0);
    
    uint16_t out;
    out = BIAS;
    spi_bus_transfer_reg16(device_handle, 0x8000 | (out <<4) , &in);
    vTaskDelay(1);
    gpio_set_level(HT1621B_CS_PIN, 1);
    vTaskDelay(1);
    gpio_set_level(HT1621B_CS_PIN, 0);
    
    out = RC256;
    spi_bus_transfer_reg16(device_handle, 0x8000 | (out <<4) , &in);
    vTaskDelay(1);
    gpio_set_level(HT1621B_CS_PIN, 1);
    vTaskDelay(1);
    gpio_set_level(HT1621B_CS_PIN, 0);
    
    out = SYSEN;
    spi_bus_transfer_reg16(device_handle, 0x8000 | (out <<4) , &in);
    vTaskDelay(1);
    gpio_set_level(HT1621B_CS_PIN, 1);
    vTaskDelay(20);
    gpio_set_level(HT1621B_CS_PIN, 0);
    
    out = LCDON;
    spi_bus_transfer_reg16(device_handle, 0x8000 | (out <<4) , &in);
    vTaskDelay(1);
    gpio_set_level(HT1621B_CS_PIN, 1);
    vTaskDelay(1);
    gpio_set_level(HT1621B_CS_PIN, 0);

    out = WDTDIS1;
    spi_bus_transfer_reg16(device_handle, 0x8000 | (out <<4), &in);
    vTaskDelay(1);
    gpio_set_level(HT1621B_CS_PIN, 1);
    vTaskDelay(1);
    gpio_set_level(HT1621B_CS_PIN, 0);
    
    out = TIMERDIS;
    spi_bus_transfer_reg16(device_handle, 0x8000 | (out <<4 ), &in);
    vTaskDelay(2);
    gpio_set_level(HT1621B_CS_PIN, 1);
    vTaskDelay(2);
    gpio_set_level(HT1621B_CS_PIN, 0);
    
    spi_bus_transfer_reg16(device_handle, 0xA07F , &in);
    spi_bus_transfer_reg16(device_handle, 0xFFFF , &in);
    spi_bus_transfer_reg16(device_handle, 0xFF80 , &in);

    gpio_set_level(HT1621B_CS_PIN, 1);
    vTaskDelay(100);
   
/*
    
    spi_bus_transfer_reg16(device_handle, 0xA07F , &in);
    spi_bus_transfer_reg16(device_handle, 0x0D6F , &in);
    spi_bus_transfer_reg16(device_handle, 0xFF80 , &in);

    gpio_set_level(HT1621B_CS_PIN, 1);
    
    */
    
//    spi_bus_device_delete(&device_handle);
//    TEST_ASSERT(device_handle == NULL);
//    spi_bus_delete(&bus_handle);
  //  TEST_ASSERT(bus_handle == NULL);
}

uint8_t digi_seg_table[11] = {0xeb, 0x0A, 0xAD, 0x8F, 0x4E,    //0~4
                              0xC7, 0xe7, 0x8a, 0xef, 0xcf,    //5~9
                              0x10 };                          //point
//uint8_t point = 0x10;

uint64_t disp_buff_bundle = 0xA000000000000000;


// pos 1~4;


void show_digital_4(uint8_t d, uint8_t pos , uint8_t show)
{
    uint64_t t_buff;
    pos-=1;
    t_buff =  (d<10)?0xEF00000000000000:0x1000000000000000;  //清除原显示的所需的mask。
    t_buff = t_buff>>(9+pos*8);
    t_buff = ~t_buff;
    disp_buff_bundle&=t_buff; //clear old;
    
    if(show)
    {
    	//make new
    	t_buff = digi_seg_table[d];
    	t_buff = t_buff<<(64-8);
    	t_buff = t_buff>>(9+pos*8);
    
    	//set buff.
    	disp_buff_bundle|=t_buff; 
    }
    
    uint16_t out16,in16,i;
    
    gpio_set_level(HT1621B_CS_PIN, 1);    
    vTaskDelay(5);
    
    gpio_set_level(HT1621B_CS_PIN, 0);

    
  
    for(i=1;i<=3;i++)
    {
    	out16 = disp_buff_bundle>>(64-16*i) &0xFFFF;
    	spi_bus_transfer_reg16(device_handle, out16 , &in16);
    }
    gpio_set_level(HT1621B_CS_PIN, 1); 
}


void app_main()
{

    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = (1ULL<<HT1621B_CS_PIN);
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    gpio_set_level(HT1621B_CS_PIN, 1);
	
    spi_bus_transfer_test();
    
    show_digital_4(1,1,TRUE);
    show_digital_4(2,2,TRUE);
    show_digital_4(3,3,TRUE);
    show_digital_4(4,4,TRUE);
    vTaskDelay(1000/portTICK_RATE_MS);
    	
        show_digital_4(5,1,TRUE);
    show_digital_4(6,2,TRUE);
    show_digital_4(7,3,TRUE);
    show_digital_4(8,4,TRUE);
    vTaskDelay(1000/portTICK_RATE_MS);
    	
        show_digital_4(9,1,TRUE);
    show_digital_4(0,2,TRUE);
    show_digital_4(1,3,TRUE);
    show_digital_4(2,4,TRUE);
    

    while(1)
    {
    	show_digital_4(SEG_POINT,1,TRUE);
    	vTaskDelay(1000/portTICK_RATE_MS);
    	
    	show_digital_4(SEG_POINT,1,FALSE);
    	vTaskDelay(1000/portTICK_RATE_MS);
    	
    }
}


