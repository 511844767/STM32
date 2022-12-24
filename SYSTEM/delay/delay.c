#include "delay.h"

static u8  fac_us=0;								   
static u16 fac_ms=0;							
	

void delay_init()
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	// 配置为外部时钟源，频率为HCLK / 8
	fac_us=SystemCoreClock/8000000;				            // 默认初始化的HCLK等于SystemCoreClock，
												            // 当HCLK为72MHz时，fac_us为9，即表示9个时钟周期耗时1μs
	fac_ms=(u16)fac_us*1000;					
}								    


static void __delay_impl(u32 fac){
	/* 记录寄存器中的旧值 */
	u32 oldLoad = SysTick->LOAD;
	u32 oldVal = SysTick->VAL;
	u32 oldCTRL = SysTick->CTRL;

	/* 开启计时并等待计时结束 */
	u32 temp;
	SysTick->LOAD=fac; 							// LOAD注意不要超过24bit（换算成时间为1864ms）
	SysTick->VAL=0x00;        					// VAL
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	// CTRL开始计时	  
	do{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		// 判断SysTick->CTRL->COUNTFLAG标志是否为1，并且计时使能打开

	/* 关闭本次计时 */
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	
	SysTick->VAL =0X00;

	/* 恢复上次计时 */
	SysTick->LOAD = oldLoad;
	SysTick->VAL = oldVal;
	SysTick->CTRL = oldCTRL; 
}


void delay_us(u32 nus)
{		
	__delay_impl(nus * fac_us);				 
}


void delay_ms(u16 nms)
{	 		  	  
	__delay_impl(nms * fac_ms);				 					  	    
}