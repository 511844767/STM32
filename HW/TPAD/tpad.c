/*************************************************************************************************/
/**
 * @brief  TPAD引脚用跳线帽连到ADC引脚，即PA1引脚，对应TIM5的通道2。
 *      用TIM5->ch2的输入捕获判断TPAD充电时间
 *     
 *      假设: TPAD电容充放电时间不超过5ms
 *
 *      定时器即使没有使能中断函数，依然需要通过中断标志位判断是否捕获成功
 *      
 *      TPAD充放电测试：
 *         未触摸时充电时间为16.5us(定时器为198)
 *         触摸时充电时间为60.9us(定时器为731)
 */
/*************************************************************************************************/
#include"stm32f10x.h"
#include"delay.h"
#include"LED.h"
#include"stdbool.h"

/* 重装载值，预分频系数，周期（5ms） */
#define __TIM_Period        (60000 - 1)
#define __TIM_Prescaler     (6 - 1)
#define __TIM_T             ((__TIM_Period + 1) * (__TIM_Prescaler + 1) / 72)


/* 初始化定时器输入捕获 */
static void TPAD_TIM5_Init(){
    /* 使能时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /* 配置PA1引脚 */
    GPIO_InitTypeDef gpioDef;
    gpioDef.GPIO_Mode   = GPIO_Mode_IN_FLOATING;
    gpioDef.GPIO_Pin    = GPIO_Pin_1;
    gpioDef.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpioDef);
    /* 配置TIM5计数器 */
    TIM_TimeBaseInitTypeDef timeBaseDef;
    timeBaseDef.TIM_CounterMode         = TIM_CounterMode_Up;   // 向上计数
    timeBaseDef.TIM_Period              = __TIM_Period;         // 重装载值
    timeBaseDef.TIM_Prescaler           = __TIM_Prescaler;      // PSC预分频系数
    timeBaseDef.TIM_ClockDivision       = TIM_CKD_DIV1;         // PWM输入采样频率分频
    timeBaseDef.TIM_RepetitionCounter   = 0X00;                 // (TIM1和TIM8的参数，暂时用不上)
    TIM_TimeBaseInit(TIM5, &timeBaseDef);
    /* 配置输入捕获 */
    TIM_ICInitTypeDef timeICDef;
    timeICDef.TIM_Channel       = TIM_Channel_2;                // TIM5的通道2
    timeICDef.TIM_ICFilter      = 0x00;                         // 滤波次数
    timeICDef.TIM_ICPolarity    = TIM_ICPolarity_Rising;        // 检测上升沿
    timeICDef.TIM_ICPrescaler   = TIM_ICPSC_DIV1;               // IC2到IC2PS信号的预分频系数
    timeICDef.TIM_ICSelection   = TIM_ICSelection_DirectTI;     // TI2映射到IC2上
    TIM_ICInit(TIM5, &timeICDef);
    /* 使能定时器 */
    TIM_Cmd(TIM5, ENABLE);
}

/* TPAD重新放电、充电 */
void TPAD_Reset(){
    /* 放电: PA1推挽输出0 */
    GPIO_InitTypeDef gpioDef;
    gpioDef.GPIO_Mode   = GPIO_Mode_Out_PP;
    gpioDef.GPIO_Pin    = GPIO_Pin_1;
    gpioDef.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpioDef);
    GPIO_ResetBits(GPIOA, GPIO_Pin_1);
    delay_ms(5);
    /* 定时器清零 */
    TIM_ClearITPendingBit(TIM5, TIM_IT_CC2 | TIM_IT_Update);
    TIM_SetCounter(TIM5, 0);
    /* 充电: PA1浮空输入 */
    gpioDef.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpioDef);
}

/* 读取一次TPAD充电时间 */
uint16_t TPAD_GetCounter(){
    /* 放、充电 */
    TPAD_Reset();
    /* 等待捕获上升沿 */
    while(TIM_GetFlagStatus(TIM5, TIM_IT_CC2) == RESET){
        if(TIM_GetFlagStatus(TIM5, TIM_IT_Update) == SET)  // 超时(5ms)
            return __TIM_Period;
    }
    return TIM_GetCounter(TIM5);
}

/* 统计TPAD充电时间 */
typedef enum{
    TPAD_Statistic_Mode_AVG,    // 统计平均值
    TPAD_Statistic_Mode_MAX     // 统计最大值
}TPAD_Statistic_Mode_t;
#define IS_TPAD_Statistic_Mode(mode) (mode == TPAD_Statistic_Mode_AVG) || (mode == TPAD_Statistic_Mode_MAX)
uint16_t TPAD_StatisticAVG(){
    /* 记录10次 */
    int i, j;
    uint16_t buffer[10];
    for(i = 0; i < 10; ++i) buffer[i] = TPAD_GetCounter();
    /* 冒泡排序 */
    uint16_t temp;
    for(i = 10 - 1; i > 0; --i){
        for(j = 0; j < i; ++j){
            if(buffer[j] > buffer[j + 1]){
                temp = buffer[j]; buffer[j] = buffer[j + 1]; buffer[j + 1] = temp;  // swap
            }
        }
    }
    /* 统计其中的6次 */
    uint32_t sum = 0;
    for(i = 2; i < 8; ++i) sum += (uint32_t)buffer[i];
    return (uint16_t)(sum / 6);
}
uint16_t TPAD_StatisticMAX(){
    /* 记录6次，记录最大值 */
    int i;
    uint16_t max = 0, temp;
    for(i = 0; i < 6; ++i){
        temp = TPAD_GetCounter();
        if(temp > max) max = temp;
    }
    return max;
}
uint16_t TPAD_StatisticCounter(TPAD_Statistic_Mode_t mode){
    assert_param(IS_TPAD_Statistic_Mode(mode));
    switch (mode)
    {
    case TPAD_Statistic_Mode_AVG:   // 平均值
        return TPAD_StatisticAVG();
    case TPAD_Statistic_Mode_MAX:   // 最大值
        return TPAD_StatisticMAX();
    }
    return 0;
}


/* 充电阈值 */
static uint16_t __TPAD_threshold = 0;


/* 初始化TPAD按键 */
void TPAD_Init(){
    /* 初始化定时器 */
    TPAD_TIM5_Init();
    /* 计算充电阈值（根据平均充电时间） */
    __TPAD_threshold = TPAD_StatisticCounter(TPAD_Statistic_Mode_AVG) + 100;
}

/* 判断是否按下 */
typedef enum{
    TPAD_Scan_Continue = 0,
    TPAD_Scan_Once,
} TPAD_Scan_Mode_t;
#define TPAD_Touched (TPAD_StatisticCounter(TPAD_Statistic_Mode_MAX) >= __TPAD_threshold)
bool TPAD_Scan(TPAD_Scan_Mode_t mode){
    static bool __tpad_empt = 1;    // 无触摸时为1，有触摸时为0

    if(mode == TPAD_Scan_Continue) __tpad_empt = 1;

    if(__tpad_empt && TPAD_Touched){  // 触摸
        __tpad_empt = 0;
        return true;
    }else if(!TPAD_Touched){          // 无触摸
        __tpad_empt = 1;
    }
    return false;
}

/* 运行例程 */
void TPAD_Run(){
    /* 初始化 */
    delay_init();
    TPAD_Init();
    LED_Init_Hal();
    /* 运行 */
    while(1){
        if(TPAD_Scan(TPAD_Scan_Once)) LED0_REVERT();
        delay_ms(10);
    }
}