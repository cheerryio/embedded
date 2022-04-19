#include "humitemp_lcd.h"

// 0-9
// 7bit dp, g, f, e, d, c, b, a 0bit
int DIGIT_VALUE_SEG[10] = {
    0x3f,
    0x06,
    0x5b,
    0x4f,
    0x66,
    0x6d,
    0x7d,
    0x07,
    0x7f,
    0x6f,
};

// a-z
// 7bit dp, g, f, e, d, c, b, a 0bit
int ALPHA_VALUE_SEG[26] = {
    0x77,
    0x7c,
    0x39,
    0x5e,
    0x79,
    0x71,
    0x3d,
    0x76,
    0x0f,
    0x0e,
    0x75,
    0x38,
    0x37,
    0x54,
    0x5c,
    0x73,
    0x67,
    0x31,
    0x49,
    0x78,
    0x3e,
    0x1c,
    0x7e,
    0x64,
    0x6e,
    0x59,
};

// 1-13
// 0 a b c f e f g dp 7
int DIGIT_POS_PIN[14][8] = {
    {-1, -1, -1, -1, -1, -1, -1, -1}, // -1
    {23, 23, 23, 22, 22, 22, 22, -1}, // 1
    {25, 25, 25, 24, 24, 24, 24, -1}, // 2
    {27, 27, 27, 26, 26, 26, 26, -1}, // 3
    {29, 29, 29, 28, 28, 28, 28, -1}, // 4
    {31, 31, 31, 30, 30, 30, 30, -1}, // 5
    {13, 13, 13, 12, 12, 12, 12, 13}, // 6
    {15, 15, 15, 14, 14, 14, 14, 15}, // 7
    {17, 17, 17, 16, 16, 16, 16, 17}, // 8
    {19, 19, 19, 18, 18, 18, 18, -1}, // 9
    {8, 8, 8, 9, 9, 9, 9, 8},         // 10
    {6, 6, 6, 7, 7, 7, 7, 6},         // 11
    {4, 4, 4, 5, 5, 5, 5, 4},         // 12
    {2, 2, 2, 3, 3, 3, 3, -1},        // 13
};

// 0 a b c d e f g dp 7
int DIGIT_POS_COM[8] = {
    1,
    2,
    3,
    4,
    3,
    1,
    2,
    4,
};

static const char* HUMITEMP_LCD_TAG="humitemp_lcd";
#define HUMITEMP_LCD_CHECK(a, str, ret_val, ...) \
    if (unlikely(!(a))) { \
        ESP_LOGE(HUMITEMP_LCD_TAG,"%s(%d): "str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        return (ret_val); \
    }

static esp_err_t write_command(humitemp_lcd_handle_t handle,uint8_t command){
    uint16_t commandBits=SPI_SWAP_DATA_TX(command<<1,9);
    esp_err_t ret=ESP_OK;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.cmd=0b100;
    t.length=9; 
    t.tx_buffer=&commandBits;
    t.flags=SPI_TRANS_VARIABLE_CMD | SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY;
    spi_transaction_ext_t transaction={
        .base=t,
        .command_bits=3,
        .address_bits=0,
        .dummy_bits=0
    };
    ret=spi_device_polling_transmit(handle->spi, (spi_transaction_t*)&transaction);  //Transmit!
    return ret;
}

static esp_err_t write_data(humitemp_lcd_handle_t handle,uint64_t addr,uint8_t* data,size_t len){
    HUMITEMP_LCD_CHECK((addr+len)<=32,"wrong addr plus len",ESP_ERR_INVALID_ARG);
    static uint8_t buffer[16];
    esp_err_t ret=ESP_OK;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    // reorder data to buffer
    for(int i=0;i<(len+1)/2;i++){
        buffer[i]=data[i*2]<<4 | data[i*2+1];
    }
    t.cmd=0b101;
    t.addr=addr;
    t.length=len*4;
    t.tx_buffer=buffer;
    t.flags=SPI_TRANS_VARIABLE_CMD | SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY;
    spi_transaction_ext_t transaction={
        .base=t,
        .command_bits=3,
        .address_bits=6,
        .dummy_bits=0
    };
    ret=spi_device_polling_transmit(handle->spi, (spi_transaction_t*)&transaction);  //Transmit!
    return ret;
}

static esp_err_t set_lcd_pin_com(humitemp_lcd_handle_t handle,uint8_t pin,uint8_t com){
    handle->mem[32-pin]=handle->mem[32-pin] | (1<<(4-com));
    return ESP_OK;
}

static esp_err_t clear_lcd_pin_com(humitemp_lcd_handle_t handle,uint8_t pin,uint8_t com){
    handle->mem[32-pin]=handle->mem[32-pin] & ~(1<<(4-com));
    return ESP_OK;
}

static int convert_ascii_to_seg(char symbol)
{
    if (symbol >= '0' && symbol <= '9')
    {
        return DIGIT_VALUE_SEG[symbol - '0'];
    }
    else if (symbol >= 'A' && symbol <= 'Z')
    {
        return ALPHA_VALUE_SEG[symbol - 'A'];
    }
    else if (symbol >= 'a' && symbol <= 'z')
    {
        return ALPHA_VALUE_SEG[symbol - 'a'];
    }
    else if (symbol == '.')
    {
        return 0x80;
    }
    else if (symbol == '-')
    {
        return 0x40;
    }
    else if (symbol == ' ')
    {
        return 0x00;
    }
    else
    {
        return 0x00;
    }
}

esp_err_t spi_setup(spi_device_handle_t *handle){
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
    ret=spi_bus_add_device(HSPI_HOST, &devcfg, handle);
    ESP_ERROR_CHECK(ret);

    return ret;
}

esp_err_t humitemp_lcd_create_handle(spi_device_handle_t spi,humitemp_lcd_handle_t* handle_ptr){
    esp_err_t ret=ESP_OK;
    humitemp_lcd_handle_t handle=(humitemp_lcd_handle_t)malloc(sizeof(humitemp_lcd_t));
    handle->spi=spi;
    *handle_ptr=handle;

    return ret;
}

esp_err_t humitemp_lcd_delete_handle(humitemp_lcd_handle_t handle){
    esp_err_t ret=ESP_OK;
    spi_bus_remove_device(handle->spi);
    free(handle);
    return ret;
}

esp_err_t humitemp_lcd_init(humitemp_lcd_handle_t handle){
    write_command(handle,BIAS4_1_3);
    write_command(handle,RC256);
    write_command(handle,SYS_EN);
    write_command(handle,LCD_ON);

    return ESP_OK;
}

// value : dp g f e d c b a
esp_err_t humitemp_lcd_set_symbol(humitemp_lcd_handle_t handle,int position,char symbol,int dp)
{
    int value=convert_ascii_to_seg(symbol);
    value|=(dp?0x80:0x00);
    for (int i = 0; i < 8; i++)
    {
        if (value & (0x1 << i))
        {
            DIGIT_POS_PIN[position][i]!=-1?set_lcd_pin_com(handle,DIGIT_POS_PIN[position][i], DIGIT_POS_COM[i]):NULL;
        }
        else
        {
            DIGIT_POS_PIN[position][i]!=-1?clear_lcd_pin_com(handle,DIGIT_POS_PIN[position][i], DIGIT_POS_COM[i]):NULL;
        }
    }
    return ESP_OK;
}

esp_err_t humitemp_lcd_flush(humitemp_lcd_handle_t handle){
    write_data(handle,0,handle->mem,32);
    return ESP_OK;
}

esp_err_t humitemp_lcd_clear_all(humitemp_lcd_handle_t handle){
    memset(handle->mem,0,sizeof(uint8_t)*32);
    return ESP_OK;
}

#define HUMITEMP_LCD_DISPALY_SYMBOL_GEN(symbol, pin, com) \
    void humitemp_lcd_set_##symbol(humitemp_lcd_handle_t handle,int on)         \
    { /* state 0 1 */                            \
        if (on)                               \
        {                                        \
            set_lcd_pin_com(handle,pin, com);           \
        }                                        \
        else                                     \
        {                                        \
            clear_lcd_pin_com(handle,pin, com);         \
        }                                        \
    }

HUMITEMP_LCD_DISPALY_SYMBOL_GEN(s6, 1, 1)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(s7, 1, 2)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(s8, 1, 3)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(kpa, 1, 4)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(rh, 2, 4)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(set, 10, 1)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(com, 10, 2)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(alert, 10, 3)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(record, 10, 4)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(h1, 11, 1)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(l1, 11, 2)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(h2, 11, 3)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(l2, 11, 4)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(s1, 19, 4)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(s2, 20, 1)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(lx, 20, 2)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(c, 20, 3)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(ppm, 20, 4)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(s3, 21, 1)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(v, 21, 2)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(ma, 21, 3)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(mpa, 21, 4)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(state, 23, 4)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(s5, 25, 4)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(col, 27, 4)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(s4, 29, 4)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(t1, 31, 4)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(t5, 32, 1)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(t4, 32, 2)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(t3, 32, 3)
HUMITEMP_LCD_DISPALY_SYMBOL_GEN(t2, 32, 4)