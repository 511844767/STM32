/**
 * @file touch.c
 * @author 511844767
 * @brief 电阻触摸屏实验
 * @version 0.1
 * @date 2023-01-03
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include"stdio.h"
#include"stdlib.h"
#include"stm32f10x.h"
#include"stm32f10x_exti.h"
#include"gpio_spi.h"
#include"delay.h"

/**
 * @brief 触摸屏检测，触摸屏的时序比较特殊
 * 
 * @param cmd 
 * @return uint16_t 
 */
extern uint16_t GPIO_SPI_Touch_Func(uint8_t cmd);

void Touch_Init(){
    /* 初始化SPI引脚 */
    GPIO_SPI_Init();
    /* 初始化Pen引脚(PF10) */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
    GPIO_InitTypeDef gpioDef;
    gpioDef.GPIO_Mode   = GPIO_Mode_IN_FLOATING;
    gpioDef.GPIO_Pin    = GPIO_Pin_10;
    gpioDef.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(GPIOF, &gpioDef);
}

typedef enum{
    Touch_State_Release = 0, 
    Touch_State_Waiting, 
    Touch_State_Pressed
} Touch_State_t;

/* 坐标可以看做定点数Q12表示 */
#define TOUCH_SHIFT 12

typedef struct{
    uint16_t x; 
    uint16_t y;
    Touch_State_t state;
} Touch_Info_t;

/* 状态机检测 */
Touch_Info_t Touch_Scan(){
    static Touch_Info_t touchInfo = {0};
    static uint8_t acc;

    #define PEN (!GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_10))
    #define READ_X() (4096 - GPIO_SPI_Touch_Func(0B11010000))
    #define READ_Y() (4096 - GPIO_SPI_Touch_Func(0B10010000))

    switch (touchInfo.state)
    {
    case Touch_State_Release:
        if(PEN){
            ++acc;
            touchInfo.state = Touch_State_Waiting;
        }
        break;
    
    case Touch_State_Waiting:
        if(PEN){
            ++acc;
            if(acc >= 10){
                touchInfo.x = READ_X();
                touchInfo.y = READ_Y();
                touchInfo.state = Touch_State_Pressed;
            }
        }else{
            acc = 0;
            touchInfo.state = Touch_State_Release;
        }
        break;

    case Touch_State_Pressed:
        if(PEN){
            touchInfo.x = READ_X();
            touchInfo.y = READ_Y();
        }else{
            touchInfo.state = Touch_State_Release;
            acc = 0;
        }
        break;

    default:    // not allowed
        break;
    }

    #undef PEN
    return touchInfo;
}

extern void USART1_Init();

void Touch_Run(){
    /* 初始化 */
    USART1_Init();
    delay_init();
    Touch_Init();

    /* 检测 */
    Touch_Info_t touchInfo;
    while(1){
        touchInfo = Touch_Scan();
        if(touchInfo.state == Touch_State_Pressed){
            printf("x = %d\ty = %d\r\n", touchInfo.x, touchInfo.y);
        }
        delay_ms(5);
    }
}
