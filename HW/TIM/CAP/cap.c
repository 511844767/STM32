#include"stm32f10x.h"
#include"stm32f10x_tim.h"
#include"stdbool.h"
#include"stdio.h"
#include"delay.h"

/* 重装载值，预分频系数，周期（0.01ms） */
#define __TIM_Period        (720 - 1)
#define __TIM_Prescaler     (1 - 1)
#define __TIM_T             ((__TIM_Period + 1) * (__TIM_Prescaler + 1) / 72)

void TIM5_Cap_Init(){
    /* 使能时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /* 配置WAKE UP按键对应的GPIOA->Pin0引脚 */
    GPIO_InitTypeDef gpioDef;
    gpioDef.GPIO_Mode   = GPIO_Mode_IPD;        // 下拉输入
    gpioDef.GPIO_Pin    = GPIO_Pin_0;
    gpioDef.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpioDef);
    /* 配置TIM5 */
    TIM_TimeBaseInitTypeDef timeBaseDef;
    timeBaseDef.TIM_CounterMode         = TIM_CounterMode_Up;   // 向上计数
    timeBaseDef.TIM_Period              = __TIM_Period;         // 重装载值
    timeBaseDef.TIM_Prescaler           = __TIM_Prescaler;      // PSC预分频系数
    timeBaseDef.TIM_ClockDivision       = TIM_CKD_DIV1;         // PWM输入采样频率分频
    timeBaseDef.TIM_RepetitionCounter   = 0X00;                 // (TIM1和TIM8的参数，暂时用不上)
    TIM_TimeBaseInit(TIM5, &timeBaseDef);
    /* 配置输入捕获 */
    TIM_ICInitTypeDef timeICDef;
    timeICDef.TIM_Channel       = TIM_Channel_1;                // TIM5的通道1
    timeICDef.TIM_ICFilter      = 0x03;                         // 滤波次数（0011：采样频率fSAMPLING=fCK_INT，N=8）
    timeICDef.TIM_ICPolarity    = TIM_ICPolarity_Rising;        // 检测上升沿
    timeICDef.TIM_ICPrescaler   = TIM_ICPSC_DIV1;               // IC1到IC1PS信号的预分频系数
    timeICDef.TIM_ICSelection   = TIM_ICSelection_DirectTI;     // TI1映射到IC1上
    TIM_ICInit(TIM5, &timeICDef);
    /* 中断初始化 */
    NVIC_InitTypeDef nvicDef;
    nvicDef.NVIC_IRQChannel                     = TIM5_IRQn;
    nvicDef.NVIC_IRQChannelCmd                  = ENABLE;
    nvicDef.NVIC_IRQChannelPreemptionPriority   = 3;
    nvicDef.NVIC_IRQChannelSubPriority          = 3;
    NVIC_Init(&nvicDef);
    TIM_ITConfig(TIM5, TIM_IT_Update | TIM_IT_CC1, ENABLE);     // 使能更新中断、通道1中断
    /* 使能定时器 */
    TIM_Cmd(TIM5, ENABLE);
}

/* 标志 */
static uint16_t polarity   = TIM_ICPolarity_Rising;    // 检测边缘（WAKE UP按下是上升沿，松开是下降沿）
static uint32_t cycles     = 0;                        // 定时器溢出次数
static uint16_t value      = 0;                        // 定时器计数
static bool captured       = false;                    // 是否捕获到了信号

void TIM5_IRQHandler(){
    if(!captured){                                                      // 如果没有捕获到信号
        if(TIM_GetITStatus(TIM5, TIM_IT_Update) == SET){                // 更新中断处理
            if(polarity == TIM_ICPolarity_Falling){                     // 如果捕获过了上升沿
                if(cycles >= 4000000 / __TIM_T){                        // 如果按下太久（4s时间），则强制捕获成功
                    captured = true; polarity = TIM_ICPolarity_Rising;
                    TIM_OC1PolarityConfig(TIM5, polarity);              // 重新配置极性
                }else{                                                  // 如果按下少于4s，则继续计时
                    ++cycles;
                }
            }
        }

        if(TIM_GetITStatus(TIM5, TIM_IT_CC1) == SET){               // 捕获中断处理
            if(polarity == TIM_ICPolarity_Rising){                  // 如果捕获到了上升沿
                cycles = 0; polarity = TIM_ICPolarity_Falling;      // 更新状态
            }else if(polarity == TIM_ICPolarity_Falling){           // 如果捕获到了下降沿
                value = TIM_GetCapture1(TIM5);                      // 记录定时器当前值
                captured = true; polarity = TIM_ICPolarity_Rising;  // 更新状态
            }
            TIM_OC1PolarityConfig(TIM5, polarity);                  // 重新配置极性
        }
    }
    TIM_ClearITPendingBit(TIM5, TIM_IT_Update | TIM_IT_CC1);    // 清除中断标志
}

/* 串口初始化 */
extern void USART1_Init();

void TIM5_Cap_Run(){
    /* 配置中断组 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    /* 初始化 */
    delay_init();
    USART1_Init();
    TIM5_Cap_Init();
    /* 工作 */
    uint32_t during;
    while (1){
        delay_ms(1);
        if(captured){   // 如果捕获到了信号，打印信号持续时间
            during = (cycles * (__TIM_Period + 1) + (value - 1)) * (__TIM_Prescaler + 1) / 72;
            printf("HIGH: %luus \r\n", during);
            captured = false;
        }
    }
}