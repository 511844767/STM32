/**
 * @file gpio_spi.c
 * @author 511844767
 * @brief 用GPIO模拟SPI信号
 * @version 0.1
 * @date 2023-01-03
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include"string.h"
#include"stm32f10x.h"
#include"gpio_spi_conf.h"
#include"delay.h"

/* 输出控制 */
#define SPI_MOSI_HIGH() GPIO_SetBits(SPI_MOSI_GPIO, SPI_MOSI_PIN)
#define SPI_MOSI_LOW()  GPIO_ResetBits(SPI_MOSI_GPIO, SPI_MOSI_PIN)
#define SPI_CS_HIGH()   GPIO_SetBits(SPI_CS_GPIO, SPI_CS_PIN)
#define SPI_CS_LOW()    GPIO_ResetBits(SPI_CS_GPIO, SPI_CS_PIN)
#define SPI_CLK_HIGH()  GPIO_SetBits(SPI_CLK_GPIO, SPI_CLK_PIN)
#define SPI_CLK_LOW()   GPIO_ResetBits(SPI_CLK_GPIO, SPI_CLK_PIN)
/* 输入读取 */
#define SPI_MISO_READ() GPIO_ReadInputDataBit(SPI_MISO_GPIO, SPI_MISO_PIN)

void GPIO_SPI_Init(){
    /* 使能时钟源 */
    RCC_APB2PeriphClockCmd(SPI_MISO_Periph | SPI_CLK_Periph | SPI_MISO_Periph | SPI_MOSI_Periph, ENABLE);
    /* 初始化GPIO */
    GPIO_InitTypeDef gpioDef;
    gpioDef.GPIO_Speed  = GPIO_Speed_50MHz;
    gpioDef.GPIO_Pin    = SPI_MISO_PIN;             // MISO输入
    gpioDef.GPIO_Mode   = GPIO_Mode_IN_FLOATING;
    GPIO_Init(SPI_MISO_GPIO, &gpioDef);
    gpioDef.GPIO_Pin    = SPI_MOSI_PIN;             // MOSI, CLK, CS输出
    gpioDef.GPIO_Mode   = GPIO_Mode_Out_PP;
    GPIO_Init(SPI_MOSI_GPIO, &gpioDef);
    gpioDef.GPIO_Pin    = SPI_CS_PIN;
    GPIO_Init(SPI_CS_GPIO, &gpioDef);
    gpioDef.GPIO_Pin    = SPI_CLK_PIN;
    GPIO_Init(SPI_CLK_GPIO, &gpioDef);
    /* 取消片选 */
    SPI_CS_HIGH();
    /* 初始化延迟 */
    delay_init();
}

__INLINE static void __GPIO_SPI_SendData(uint8_t* src, uint8_t size){
    /* 初始状态 */
    SPI_MOSI_LOW();
    SPI_CLK_LOW();

    /* 发送数据 */
    SPI_CS_LOW();
    delay_us(1);    // >= 100ns
    /* 遍历每个bit */
    for(uint8_t i = 0; i < 8 * size; ++i){
        SPI_CLK_LOW();
        if(src[i / 8] & (0B10000000 >> (i % 8)))   // 先发送高位
            SPI_MOSI_HIGH();
        else
            SPI_MOSI_LOW();
        SPI_CLK_HIGH();
        delay_us(1);    // >= 200ns
    }
    SPI_CS_HIGH();
}
void GPIO_SPI_SendData(void* src, uint8_t size){
    __GPIO_SPI_SendData((uint8_t*)src, size);
}


__INLINE static void __GPIO_SPI_RecieveData(uint8_t* dst, uint8_t size){
    /* 初始状态 */
    SPI_MOSI_LOW();
    SPI_CLK_LOW();
    memset(dst, 0, size);

    /* 接收数据 */
    SPI_CS_LOW();
    delay_us(1);    // >= 100ns
    /* 遍历每个bit */
    for(uint8_t i = 0; i < 8 * size; ++i){
        SPI_CLK_HIGH();
        delay_us(1);    // >= 200ns
        if(SPI_MISO_READ()){
            dst[i / 8] |= 0B10000000 >> (i % 8);
        }
        SPI_CLK_LOW();
        delay_us(1);    // >= 200ns
    }
    SPI_CS_HIGH();
}
void GPIO_SPI_RecieveData(void* dst, uint8_t size){
    __GPIO_SPI_RecieveData((uint8_t*)dst, size);
}

uint16_t GPIO_SPI_Touch_Func(uint8_t cmd){
    /* 初始状态 */
    SPI_MOSI_LOW();
    SPI_CLK_LOW();
    SPI_CS_LOW();
    delay_111ns();    // >= 100ns

    /* 发送命令 */
    for(uint8_t i = 0; i < 8; ++i){
        SPI_CLK_LOW();
        if(cmd & (0B10000000 >> (i)))   // 先发送高位
            SPI_MOSI_HIGH();
        else
            SPI_MOSI_LOW();
        delay_208ns();    // >= 200ns
        SPI_CLK_HIGH();
        delay_208ns();    // >= 200ns
    }

    /* 等待BUSY */
    SPI_CLK_LOW();
    delay_208ns();    // >= 200ns
    SPI_CLK_HIGH();
    delay_208ns();    // >= 200ns
    SPI_CLK_LOW();

    /* 接收返回值 */
    uint16_t data = 0;
    for(uint8_t i = 0; i < 12; ++i){
        SPI_CLK_HIGH();
        delay_208ns();    // >= 200ns
        if(SPI_MISO_READ()){
            data |= 0X800 >> (i);   // 先接收高位
        }
        SPI_CLK_LOW();
        delay_208ns();    // >= 200ns
    }

    /* 取消片选 */
    SPI_CS_HIGH();

    return data;
}