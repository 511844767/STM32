#include"stm32f10x.h"
#include"stm32f10x_iwdg.h"
#include"delay.h"

void IWDG_Init(uint8_t IWDG_Prescaler, uint16_t Reload){
    /* 使PR和RLR寄存器可写 */
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

    /* 设置预分频系数 */
    IWDG_SetPrescaler(IWDG_Prescaler);

    /* 设置重装载值 */
    IWDG_SetReload(Reload);

    /* 装载一次计时器 */
    IWDG_ReloadCounter();

    /* 使能看门狗 */
    IWDG_Enable();
}

/* KEY与LED灯寄存器 */
#define KEY0  GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4)
#define KEY1  GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3)
#define KEYUP GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)
#define LED0  PBout(5)
#define LED1  PEout(5)
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
extern void LED_Init_Hal(void);
extern void KEY_Init();
extern KEY_STATUS_t KEY_Scan(KEY_SCAN_MODE_t mode);


void IWDG_Run(){
    /* 初始化 */
    IWDG_Init(IWDG_Prescaler_32, 1250);     // 看门狗定时1s
    LED_Init_Hal();                         // LED初始化并亮灯
    delay_init();
    KEY_Init();

    /* 复位后等0.5s关灯 */
    delay_ms(500);
    LED0 = 0;

    while(1){
        if(KEY_Scan(KEY_SCAN_ONCE) == KEY0_PRESS){  // 按键喂狗
            IWDG_ReloadCounter();
        }
    }
}