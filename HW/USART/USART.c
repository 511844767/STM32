#include"stm32f10x.h"
#include"stm32f10x_usart.h"
#include"stdio.h"
#include"stdlib.h"
#include"string.h"
#include"delay.h"
#include"math.h"

/* 将printf重定向到USART1 */
int _write(int fd, char *ptr, int len)  
{  
    int i;
    for(i = 0; i < len; ++i){
        /* 等待上一个数据发送完成 */
        while((USART1->SR & 0X40) == 0);
        /* 发送当前数据 */
        USART_SendData(USART1, ptr[i]);
    }
    return len;
}

/* 串口初始化 */
void USART1_Init(){
    /* 关闭printf缓冲，否则printf不会实时发送到串口 */
    setvbuf(stdout, NULL, _IONBF, 0);

    /* USART1时钟使能 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    /* 配置GPIO复用为USART1 */
    GPIO_InitTypeDef gpioAPin9Def = { GPIO_Pin_9, GPIO_Speed_10MHz, GPIO_Mode_AF_PP };
    GPIO_InitTypeDef gpioAPin10Def = { GPIO_Pin_10, GPIO_Speed_10MHz, GPIO_Mode_IN_FLOATING };
    GPIO_Init(GPIOA, &gpioAPin9Def);
    GPIO_Init(GPIOA, &gpioAPin10Def);

    /* 初始化USART1 */
    USART_InitTypeDef usart1Def = { 
        115200, 
        USART_WordLength_8b,
        USART_StopBits_1,  
        USART_Parity_No, 
        USART_Mode_Tx | USART_Mode_Rx, 
        USART_HardwareFlowControl_None,  
    };
    USART_Init(USART1, &usart1Def);

    /* 使能USART1 */
    USART_Cmd(USART1, ENABLE);

    /* 使能USART1 RXNE状态寄存器中断（接收中断） */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    NVIC_InitTypeDef nvicDef = { USART1_IRQn, 0XF, 0X0, ENABLE };
    NVIC_Init(&nvicDef);
}

/* 
    中断服务函数，在startup_stm32f103xe.s中定义了接口
    值得注意的是，如果接收了字符串str，则USART1_IRQHandler会被调用len(str)次
*/
void USART1_IRQHandler(){
    if(USART_GetITStatus(USART1, USART_IT_RXNE)){   // 判断是否为接收中断
        u16 res = USART_ReceiveData(USART1);        // 接收串口数据
        USART_SendData(USART1, res);                // 串口发送数据
    }
}


void USART1_Run(){
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    USART1_Init();
    delay_init();
    int t = 0;
    while(1){
        delay_ms(1000);
        printf("%d\r\n", t++);
    }
}