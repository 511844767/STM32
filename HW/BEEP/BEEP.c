#include "stm32f10x.h"
#include "sys.h"
#include"delay.h"

void BEEP_Init(){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef gpioDef = {GPIO_Pin_8, GPIO_Speed_50MHz, GPIO_Mode_Out_PP};
    GPIO_Init(GPIOB, &gpioDef);
}

void BEEP_Bit(){
    BEEP_Init();
    delay_init();

    PBout(8) = 1;
    delay_ms(500);
    PBout(8) = 0;  
    delay_ms(500);
    PBout(8) = 1;
    delay_ms(1000);
    PBout(8) = 0;
}