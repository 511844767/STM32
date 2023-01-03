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

/* 电压参考值 */
typedef struct{
    float x;
    float y;
} Touch_Ref_t;
static Touch_Ref_t reference = {0};


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

    /* 检测空状态下的电压参考值 */
    {
        /* 记录10次 */
        int i, j;
        uint16_t xBuffer[10], yBuffer[10];
        for(i = 0; i < 10; ++i) {
            xBuffer[i] = GPIO_SPI_Touch_Func(0B11010000);
            yBuffer[i] = GPIO_SPI_Touch_Func(0B10010000);
        }

        /* 冒泡排序 */
        uint16_t temp;
        for(i = 10 - 1; i > 0; --i){
            for(j = 0; j < i; ++j){
                if(xBuffer[j] > xBuffer[j + 1]){
                    temp = xBuffer[j]; xBuffer[j] = xBuffer[j + 1]; xBuffer[j + 1] = temp;  // swap
                }
                if(yBuffer[j] > yBuffer[j + 1]){
                    temp = yBuffer[j]; yBuffer[j] = yBuffer[j + 1]; yBuffer[j + 1] = temp;  // swap
                }
            }
        }
        /* 统计其中的6次 */
        float xSum = 0, ySum = 0;
        for(i = 2; i < 8; ++i) {
            xSum += xBuffer[i];
            ySum += yBuffer[i];
        }

        reference.x = xSum / 6;
        reference.y = ySum / 6;
    }
}

typedef enum{
    Touch_State_Release = 0, 
    Touch_State_Waiting, 
    Touch_State_Pressed
} Touch_State_t;

typedef struct{
    float x; 
    float y;
    Touch_State_t state;
} Touch_Info_t;

/* 状态机检测 */
Touch_Info_t Touch_Scan(){
    static Touch_Info_t touchInfo = {0};
    static uint8_t acc;

    #define PEN (!GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_10))
    #define READ_X() GPIO_SPI_Touch_Func(0B11010000) / (4095 - reference.x)
    #define READ_Y() (reference.y - GPIO_SPI_Touch_Func(0B10010000)) / reference.y

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
            printf("x = %.3f\ty = %.3f\r\n", touchInfo.x, touchInfo.y);
        }
        delay_ms(5);
    }
}
