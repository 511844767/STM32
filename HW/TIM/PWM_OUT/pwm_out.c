#include"stm32f10x.h"
#include"stm32f10x_tim.h"
#include"LED.h"
#include"delay.h"

void TIM3_PWMOut_Init(uint16_t TIM_Period, uint16_t TIM_Prescaler, uint16_t TIM_Pulse){
    /* 使能时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);    // TIM3定时器
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);   // LED0在GPIOB上
    /* 配置GPIOB->Pin5 */
    GPIO_InitTypeDef gpioDef;
    gpioDef.GPIO_Pin    = GPIO_Pin_5;
    gpioDef.GPIO_Speed  = GPIO_Speed_50MHz;
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP;                  // 复用推挽输出
	GPIO_Init(GPIOB, &gpioDef);
    /* TIM3定时器部分重映射 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);    // 使能重映射时钟
    GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);
    /* 配置TIM3 */
    TIM_TimeBaseInitTypeDef timeBaseDef;
    timeBaseDef.TIM_CounterMode         = TIM_CounterMode_Down; // 向下计数
    timeBaseDef.TIM_Period              = TIM_Period;           // 重装载值
    timeBaseDef.TIM_Prescaler           = TIM_Prescaler;        // PSC预分频系数
    timeBaseDef.TIM_ClockDivision       = TIM_CKD_DIV1;         // 采样频率分频（暂时用不上）
    timeBaseDef.TIM_RepetitionCounter   = 0X00;                 // PWM输入相关系数（暂时用不上）
    TIM_TimeBaseInit(TIM3, &timeBaseDef);
    TIM_OCInitTypeDef timeOCDef;
    timeOCDef.TIM_OCMode        = TIM_OCMode_PWM2;              // PWM模式2，计数器大于阈值时为有效电平
    timeOCDef.TIM_OCPolarity    = TIM_OCPolarity_High;          // PWM极性，高电平为有效电平
    timeOCDef.TIM_OutputState   = TIM_OutputState_Enable;       // 使能输出
    timeOCDef.TIM_Pulse         = TIM_Pulse;                    // 用这个也可以初始化阈值（预装载值）
    TIM_OC2Init(TIM3, &timeOCDef);
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);           // 使能阈值（预装载值）
    TIM_Cmd(TIM3, ENABLE);                                      // 使能定时器
}

void TIM3_PWMOut_Run(){
    /* 初始化 */
    LED_Init_Hal();
    delay_init();
    uint16_t TIM_Pulse = 450;
    TIM3_PWMOut_Init(900 - 1, 0, TIM_Pulse);  // PWM频率为72MHz / 900 = 80KHz，占空比为(900 - 450) / 900 = 0.5
    /* 随着时间改变占空比（1~0.66之间） */
    u8 dir = 0;
    while(1){
        delay_ms(1);
        if(TIM_Pulse <= 0)   dir = 1;      // 占空比下降
        if(TIM_Pulse >= 900) dir = 0;      // 占空比上升
        if(dir) ++TIM_Pulse;
        else    --TIM_Pulse;
        TIM_SetCompare2(TIM3, TIM_Pulse);
    }
}