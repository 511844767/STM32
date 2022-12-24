#include "stm32f10x.h"
#include"delay.h"

typedef enum{
    KEY_NONE_PRESS = 0,
    KEY0_PRESS, 
    KEY1_PRESS, 
    KEYUP_PRESS, 
} KEY_STATUS_t;

typedef enum{
    KEY_SCAN_CONTINUE = 0,
    KEY_SCAN_ONCE,
} KEY_SCAN_MODE_t;

/* 配置时钟、输入模式 */
void KEY_Init(){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOE, ENABLE);

    GPIO_InitTypeDef KEY01Def = 
        {GPIO_Pin_3 | GPIO_Pin_4, GPIO_Speed_50MHz, GPIO_Mode_IPU}; // KEY0, KEY1: GPIO_Mode_IPU上拉输入，按钮断开时为高电平
    GPIO_Init(GPIOE, &KEY01Def);    // GPIO_Init中for循环遍历了所有的Pin脚，所以可以在GPIO_InitTypeDef用或运算同时传入多个Pin脚
    GPIO_InitTypeDef KEYUPDef = 
        {GPIO_Pin_0, GPIO_Speed_50MHz, GPIO_Mode_IPD};  // KEY UP: GPIO_Mode_IPD下拉输入，按钮断开时为低电平
    GPIO_Init(GPIOA, &KEYUPDef);
}

/**
  * @brief  按键扫描
  * @param  mode: 
  *             KEY_SCAN_CONTINUE表示若按钮按下，每次扫描都返回按下状态
  *             KEY_SCAN_ONCE表示若按钮按下，只返回一次按下状态，如果不松手，下一次返回KEY_NONE_PRESS
  * @retval 按钮状态
  */
KEY_STATUS_t KEY_Scan(KEY_SCAN_MODE_t mode){
    static u8 __key_up = 1;   // 没有按键按下时为1，有按键按下时为0

    if(mode == KEY_SCAN_CONTINUE) __key_up = 1;

#define KEY0  GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4)
#define KEY1  GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3)
#define KEYUP GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)
    if(__key_up && (KEY0 == 0 || KEY1 == 0 || KEYUP == 1)){ // 按下
        delay_ms(10);       // 去抖动
        __key_up = 0;
        if(KEY0  == 0) return KEY0_PRESS;
        if(KEY1  == 0) return KEY1_PRESS;
        if(KEYUP == 1) return KEYUP_PRESS;
    }else if(KEY0 == 1 && KEY1 == 1 && KEYUP == 0){         // 松开
        __key_up = 1;     
    }
    return KEY_NONE_PRESS;
}

void LED_Init_Hal(void);

/* 运行例程 */
void KEY_RUN(){
    LED_Init_Hal();
    KEY_Init();
    delay_init();

    // LED灯位映射
#define LED0 PBout(5)
#define LED1 PEout(5)

    while(1){
        KEY_STATUS_t key = KEY_Scan(KEY_SCAN_ONCE);
        if(key != KEY_NONE_PRESS){
            switch (key)
            {
            case KEY0_PRESS:    // LED0翻转
                LED0 = !LED0;
                break;
            case KEY1_PRESS:    // LED1翻转
                LED1 = !LED1;
                break;
            case KEYUP_PRESS:   // LED0, LED1一起翻转
                LED0 = !LED0;
                LED1 = !LED1;
                break;
            default:
                break;
            }
        }
    }
}