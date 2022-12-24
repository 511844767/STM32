#include"stm32f10x.h"
#include"stm32f10x_dma.h"
#include"stdio.h"

/**
 * @brief 用DMA将数据从内存拷贝到串口中
 * 
 */

/* 内存地址 */
static char buffer[500];

void DMA_USART_Init(){
    /* 使能DMA时钟 */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    /* 配置DMA通道 */
    DMA_InitTypeDef dmaDef;
    dmaDef.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);  // 串口数据寄存器地址
    dmaDef.DMA_MemoryBaseAddr = (uint32_t)buffer; // 内存地址
    dmaDef.DMA_DIR = DMA_DIR_PeripheralDST; // 传输方向，M->P
    dmaDef.DMA_BufferSize = 500;    // 数据大小(CNDTR寄存器)
    dmaDef.DMA_PeripheralInc = DMA_PeripheralInc_Disable;    // 地址自增
    dmaDef.DMA_MemoryInc = DMA_MemoryInc_Enable;    // 地址自增
    dmaDef.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;    // 数据大小
    dmaDef.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;    // 数据大下
    dmaDef.DMA_Mode = DMA_Mode_Normal;  // 传输模式（单次传输/循环传输）
    dmaDef.DMA_Priority = DMA_Priority_Medium;    // 传输优先级
    dmaDef.DMA_M2M = DMA_M2M_Disable;   // 关闭M->M的传输方向
    DMA_Init(DMA1_Channel4, &dmaDef);   // USART_TX对应通道4
    /* 使能DMA通道 */
    DMA_SetCurrDataCounter(DMA1_Channel4, 500); // 在DMA_Cmd被DISABLE的情况下，可以用这个函数修改DMA_BufferSize的大小
    DMA_Cmd(DMA1_Channel4, ENABLE);
    // printf("DMA Buffer Size: %u\r\n", DMA_GetCurrDataCounter(DMA1_Channel4));    // 获取DMA_BufferSize的大小
}

/* 串口初始化函数 */
extern void USART1_Init();

void DMA_USART_Run(){
    /* 初始化寄存器 */
    USART1_Init();
    DMA_USART_Init();
    /* 初始化数据 */
    int i;
    for(i = 0; i < 500; ++i){
        // buffer[i] = i % 128;
        buffer[i] = 'A';
    }
    /* 发送数据 */
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
    while (1);
}