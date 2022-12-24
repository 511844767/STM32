#include"stm32f10x.h"
#include"delay.h"
#include"stm32f10x_exti.h"


void EXTIX_Init(){
    /* 使能AFIO寄存器的时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /* 中断线映射 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0); // GPIOA_Pin0映射到中断线0
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource3); // GPIOE_Pin3映射到中断线3
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource4); // GPIOE_Pin4映射到中断线4

    /* 初始化中断线 */
    EXTI_InitTypeDef exti0Def;
    exti0Def.EXTI_Line    = EXTI_Line0;
    exti0Def.EXTI_LineCmd = ENABLE;
    exti0Def.EXTI_Mode    = EXTI_Mode_Interrupt;
    exti0Def.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&exti0Def);

    EXTI_InitTypeDef exti3Def;
    exti3Def.EXTI_Line    = EXTI_Line3;
    exti3Def.EXTI_LineCmd = ENABLE;
    exti3Def.EXTI_Mode    = EXTI_Mode_Interrupt;
    exti3Def.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&exti3Def);

    EXTI_InitTypeDef exti4Def;
    exti4Def.EXTI_Line    = EXTI_Line4;
    exti4Def.EXTI_LineCmd = ENABLE;
    exti4Def.EXTI_Mode    = EXTI_Mode_Interrupt;
    exti4Def.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&exti4Def);

    /* 配置中断优先级 */
    NVIC_InitTypeDef nvic0Def;
    nvic0Def.NVIC_IRQChannel = EXTI0_IRQn;
    nvic0Def.NVIC_IRQChannelCmd = ENABLE;
    nvic0Def.NVIC_IRQChannelPreemptionPriority = 2;
    nvic0Def.NVIC_IRQChannelSubPriority = 2;
    NVIC_Init(&nvic0Def);
    
    NVIC_InitTypeDef nvic3Def;
    nvic3Def.NVIC_IRQChannel = EXTI3_IRQn;
    nvic3Def.NVIC_IRQChannelCmd = ENABLE;
    nvic3Def.NVIC_IRQChannelPreemptionPriority = 2;
    nvic3Def.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&nvic3Def);

    NVIC_InitTypeDef nvic4Def;
    nvic4Def.NVIC_IRQChannel = EXTI4_IRQn;
    nvic4Def.NVIC_IRQChannelCmd = ENABLE;
    nvic4Def.NVIC_IRQChannelPreemptionPriority = 2;
    nvic4Def.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&nvic4Def);
}


/* KEY与LED灯寄存器 */
#define KEY0  GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4)
#define KEY1  GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3)
#define KEYUP GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)
#define LED0  PBout(5)
#define LED1  PEout(5)


/* 中断服务函数 */
void EXTI0_IRQHandler(){
    delay_ms(10);
    if(KEYUP == 1){
        LED0 = !LED0;
        LED1 = !LED1;
    }
    /* 中断服务函数退出前要手动清除中断标志位 */
    EXTI_ClearITPendingBit(EXTI_Line0);
}
void EXTI3_IRQHandler(){
    delay_ms(10);
    if(KEY1 == 0){
        LED1 = !LED1;
    }
    /* 中断服务函数退出前要手动清除中断标志位 */
    EXTI_ClearITPendingBit(EXTI_Line3);
}
void EXTI4_IRQHandler(){
    delay_ms(10);
    if(KEY0 == 0){
        LED0 = !LED0;
    }
    /* 中断服务函数退出前要手动清除中断标志位 */
    EXTI_ClearITPendingBit(EXTI_Line4);
}

/* 初始化函数 */
extern void KEY_Init();
extern void LED_Init_Hal(void);


void EXTI_Run(){
    /* 初始化中断优先级分组 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* 初始化延迟 */
    delay_init();

    /* 初始化LED */
    LED_Init_Hal();

    /* 初始化按键IO脚 */
    KEY_Init();

    /* 初始化外部中断 */
    EXTIX_Init();

    /* 等待 */
    while(1);
}