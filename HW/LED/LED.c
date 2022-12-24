#include"stm32f10x.h"
#include"delay.h"
#include"LED.h"

/*-----------------------------初始化--------------------------------------*/

void LED_Init_Hal(void){
	/* 初始化GPIOx时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOE, ENABLE);
	
	/* 配置GPIO模式  */
	GPIO_InitTypeDef gpioDef[2] = {
		{GPIO_Pin_5, GPIO_Speed_50MHz, GPIO_Mode_Out_PP}, 
		{GPIO_Pin_5, GPIO_Speed_50MHz, GPIO_Mode_Out_PP}
	};
	GPIO_Init(GPIOB, &gpioDef[0]);
	GPIO_Init(GPIOE, &gpioDef[1]);

	/* LED引脚输出高电平 */
	GPIO_SetBits(GPIOB, GPIO_Pin_5);
	GPIO_SetBits(GPIOE, GPIO_Pin_5);
}


void LED_Init_Register(void){
	/* 初始化GPIOx时钟 */
	RCC->APB2ENR |= 1 << 3;	// B
	RCC->APB2ENR |= 1 << 6;	// E

	/* 配置GPIO模式  */
	GPIOB->CRL = (GPIOB->CRL & 0XFF0FFFFF) | 3 << 20;
	GPIOE->CRL = (GPIOE->CRL & 0XFF0FFFFF) | 3 << 20;

	/* LED引脚输出高电平 */
	GPIOB->ODR |= 1 << 5;
	GPIOE->ODR |= 1 << 5;
}

/*-----------------------------运行--------------------------------------*/

void LED_HAL(){
    LED_Init_Hal();
    delay_init();

    while (1){
        GPIO_SetBits(GPIOB, GPIO_Pin_5);
	    GPIO_SetBits(GPIOE, GPIO_Pin_5);
        delay_ms(500);

        GPIO_ResetBits(GPIOB, GPIO_Pin_5);
	    GPIO_ResetBits(GPIOE, GPIO_Pin_5);
        delay_ms(500);
    }
}

void LED_Register(){
    LED_Init_Register();
    delay_init();

    while(1){
        GPIOB->ODR |= 1<<5;
        GPIOE->ODR |= 1<<5;
        delay_ms(500);

        GPIOB->ODR &= ~(1<<5);
        GPIOE->ODR &= ~(1<<5);
        delay_ms(500);
    }
}

#define LED0  PBout(5)
#define LED1  PEout(5)

void LED_Bit(){
	LED_Init_Register();
	delay_init();

	while(1){
		PBout(5) = 1;
		PEout(5) = 1;
		delay_ms(500);
		PBout(5) = 0;
		PEout(5) = 0;
		delay_ms(500);
	}
}
