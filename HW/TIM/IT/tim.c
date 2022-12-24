#include"stm32f10x.h"
#include"LED.h"
#include"delay.h"
#include"stm32f10x_tim.h"

void TIM3_Init(uint16_t TIM_Period, uint16_t TIM_Prescaler){
    /* 使能定时器器时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    /* 配置定时器 */
    TIM_TimeBaseInitTypeDef timeBaseDef;
    timeBaseDef.TIM_CounterMode         = TIM_CounterMode_Down; // 向下计数
    timeBaseDef.TIM_Period              = TIM_Period;           // 重装载值
    timeBaseDef.TIM_Prescaler           = TIM_Prescaler;        // PSC预分频系数
    timeBaseDef.TIM_ClockDivision       = TIM_CKD_DIV1;         // 采样频率分频（暂时用不上）
    timeBaseDef.TIM_RepetitionCounter   = 0X00;                 // PWM输入相关系数（暂时用不上）
    TIM_TimeBaseInit(TIM3, &timeBaseDef);
    /* 使能更新中断 */
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    /* 配置中断 */
    NVIC_InitTypeDef nvicDef;
    nvicDef.NVIC_IRQChannel = TIM3_IRQn;
    nvicDef.NVIC_IRQChannelCmd = ENABLE;
    nvicDef.NVIC_IRQChannelPreemptionPriority = 3;
    nvicDef.NVIC_IRQChannelSubPriority = 3;
    NVIC_Init(&nvicDef);
    /* 使能定时器 */
    TIM_Cmd(TIM3, ENABLE);
}

void TIM3_IRQHandler(){
    /* 判断中断类型 */ 
    if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET){   
        /* LED0翻转 */
        LED0_REVERT();
        /* 清除中断标志 */
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}

/* 定时器中断实验 */
void TIM3_Run(){
    /* 中断优先级分组 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    /* 初始化 */
    LED_Init_Hal();
    delay_init();
    TIM3_Init(5625 - 1, 12800 - 1); // 重装载值为5624，预分频系数为12799，在时钟源频率为72MHz下定时1s
    /* 等待 */
    while(1);
}