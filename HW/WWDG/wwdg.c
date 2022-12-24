#include"stm32f10x.h"
#include"stm32f10x_wwdg.h"
#include"sys.h"
#include"delay.h"
#include"LED.h"

/* 喂狗重装载值 */
static uint8_t Counter = 0X7F;

void WWDG_Init(uint32_t WWDG_Prescaler, uint8_t WindowValue, uint8_t __Counter){
    /* 重装载值最大为0X7F */
    Counter = __Counter & 0X7F;
    /* 使能看门狗时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);
    /* 设置预分频系数 */
    WWDG_SetPrescaler(WWDG_Prescaler);
    /* 设置窗口值 */
    WWDG_SetWindowValue(WindowValue);
    /* 使能窗口看门狗 */
    WWDG_Enable(Counter);
    /* 清除计时结束前中断标志 */
    WWDG_ClearFlag();
    /* 配置看门狗中断 */
    NVIC_InitTypeDef nvicDef;
    nvicDef.NVIC_IRQChannel = WWDG_IRQn;
    nvicDef.NVIC_IRQChannelCmd = ENABLE;
    nvicDef.NVIC_IRQChannelPreemptionPriority = 2;
    nvicDef.NVIC_IRQChannelSubPriority = 3;
    NVIC_Init(&nvicDef);
    /* 使能看门狗中断 */
    WWDG_EnableIT();
}

/* 中断服务函数，在复位前触发 */
void WWDG_IRQHandler(){
    /* 喂狗 */
    WWDG_SetCounter(Counter);
    /* 清除计时结束前中断标志 */
    WWDG_ClearFlag();
}

void WWDG_Run(){
    /* 配置中断优先级组 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    /* 初始化 */
    LED_Init_Hal();
    delay_init();
    WWDG_Init(WWDG_Prescaler_8, 0X7F, 0X7F);    // 窗口值为0X7F，复位值为0X7F
    /* 复位后等0.5s亮灯 */
	delay_ms(500);
    LED0_ON();
    while(1);
}