/**
 * @file SDCard_eval.h
 * @author 511844767
 * @brief 移植SD库的底层函数实现
 * @version 0.1
 * @date 2022-12-31
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef SD_LowLevel_Implement_H__
#define SD_LowLevel_Implement_H__
#include"stm32f10x.h"

/**
 * @brief SDIO初始化参数，STM32F1的SDIO的时钟为HCLK（默认72MHz）
 * 
 */
#define SDIO_INIT_CLK_DIV       178     // 识别阶段，分频到400KHz
#define SDIO_TRANSFER_CLK_DIV   1       // 传输阶段，默认模式下分频到24MHz
static uint32_t SD_BlockSize    =  0;   // 记录SD卡的block大小，对于SDHC, SDXC访问不同地址需要设置不同的大小

/**
 * @brief 实现底层初始化
 * 
 */
void SD_LowLevel_Init(){
    /* 配置SD卡中断 */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    /* 配置SDIO的GPIO引脚 */
    /* 使能时钟源 */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SDIO, ENABLE);                              // 使能SDIO外设时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);                              // 使能DMA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);    // 使能对应GPIO引脚时钟
    /* 配置引脚 */
    GPIO_InitTypeDef gpioDef;
    gpioDef.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioDef.GPIO_Speed = GPIO_Speed_50MHz;
    gpioDef.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;   // SDIO_D0, D1, D2, D3, CK
    GPIO_Init(GPIOC, &gpioDef);
    gpioDef.GPIO_Pin = GPIO_Pin_2;  // SDIO_CMD
    GPIO_Init(GPIOD, &gpioDef);
}


/**
 * @brief 解初始化
 * 
 */
void SD_LowLevel_DeInit(){
    /* 失能SDIO */
    SDIO_ClockCmd(DISABLE);

    /* 关闭SDIO电源 */
    SDIO_SetPowerState(SDIO_PowerState_OFF);

    /* 清除SDIO寄存器 */
    SDIO_DeInit();

    /* 关闭SDIO时钟源 */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SDIO, DISABLE);

    /* 重置引脚为浮空输入 */
    GPIO_InitTypeDef gpioDef;
    gpioDef.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
    gpioDef.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpioDef.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &gpioDef);
    gpioDef.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOD, &gpioDef);

    /* 关闭DMA */
    DMA_ClearFlag(DMA2_FLAG_TC4 | DMA2_FLAG_TE4 | DMA2_FLAG_HT4 | DMA2_FLAG_GL4);
    DMA_Cmd(DMA2_Channel4, DISABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, DISABLE);
    DMA_DeInit(DMA2_Channel4);
}


static __INLINE void __SD_DMA_Conf_Impl(uint32_t* ptr, uint32_t size, uint32_t DMA_DIR){
    /* 使能时钟源 */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);          // 使能DMA时钟
    /* 清除DMA标志 */
    DMA_ClearFlag(DMA2_FLAG_TC4 | DMA2_FLAG_TE4 | DMA2_FLAG_HT4 | DMA2_FLAG_GL4);
    /* 失能DMA */
    DMA_Cmd(DMA2_Channel4, DISABLE);
    /* 配置DMA通道 */
    DMA_InitTypeDef dmaDef;
    dmaDef.DMA_PeripheralBaseAddr = (uint32_t)&SDIO->FIFO;      // FIFO数据寄存器
    dmaDef.DMA_MemoryBaseAddr     = (uint32_t)ptr;              // 内存地址
    dmaDef.DMA_DIR                = DMA_DIR;                    // 传输方向
    dmaDef.DMA_BufferSize         = size / 4;                   // 数据大小(CNDTR寄存器)
    dmaDef.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;  // 地址自增
    dmaDef.DMA_MemoryInc          = DMA_MemoryInc_Enable;       // 地址自增
    dmaDef.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;// 数据大小
    dmaDef.DMA_MemoryDataSize     = DMA_MemoryDataSize_Word;    // 数据大小
    dmaDef.DMA_Mode               = DMA_Mode_Normal;            // 传输模式（单次传输/循环传输）
    dmaDef.DMA_Priority           = DMA_Priority_High;          // 传输优先级
    dmaDef.DMA_M2M                = DMA_M2M_Disable;            // 关闭M->M的传输方向
    DMA_Init(DMA2_Channel4, &dmaDef);                           // USART_TX对应通道4
    /* 使能DMA通道 */
    DMA_Cmd(DMA2_Channel4, ENABLE);
}


/**
  * @brief 配置SDIO对应的DMA4（P->M方向）
  * @param dst 从FIFO中读取数据到dst内存中
  * @param size 数据大小，单位为Byte
  * @retval None
 */
void SD_LowLevel_DMA_RxConfig(uint32_t* dst, uint32_t size){
    __SD_DMA_Conf_Impl(dst, size, DMA_DIR_PeripheralSRC);
}

/**
  * @brief 配置SDIO对应的DMA4（M->P方向）
  * @param src 从src内存中发送数据到FIFO中
  * @param size 数据大小，单位为Byte
  * @retval None
 */
void SD_LowLevel_DMA_TxConfig(uint32_t* src, uint32_t size){
    __SD_DMA_Conf_Impl(src, size, DMA_DIR_PeripheralDST);
}

/**
 * @brief 查询DMA传输状态
 * @return DMA传输状态
 */
__INLINE FlagStatus SD_DMAEndOfTransferStatus(void){
  return DMA_GetFlagStatus(DMA2_FLAG_TC4);
}

/**
 * @brief 检查BlockSize大小，如果更新了则发送CMD16更新
 * @return SD_Error 
 */
#define SD_CHECK_BLOCKSIZE(BlockSize){ \
  /* 如果BlockSize和上次设定的不一样，则发送CMD16命令进行更新 */ \
  /* 如果不更新，程序会卡死在DMA等待循环 */ \
  if(BlockSize != SD_BlockSize){ \
    /* 检查BlockSize大小是否符合规范 */ \
    if(BlockSize == 0 && BlockSize != 2048 && ((BlockSize & (BlockSize - 1)) != 0)){ \
      return SD_INVALID_PARAMETER; \
    }  \
     \
    SDIO_CmdInitStructure.SDIO_Argument = BlockSize; \
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN; \
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short; \
    SDIO_CmdInitStructure.SDIO_Wait     = SDIO_Wait_No; \
    SDIO_CmdInitStructure.SDIO_CPSM     = SDIO_CPSM_Enable; \
    SDIO_SendCommand(&SDIO_CmdInitStructure); \
     \
    errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN); \
 \
    if (SD_OK != errorstatus) \
    { \
      return(errorstatus); \
    } \
 \
    SD_BlockSize = BlockSize; \
  } \
}

/**
 * @brief SDIO中断服务函数
 * 
 */
void SDIO_IRQHandler(void){
  SD_ProcessIRQSrc();
}
#endif