/**
 * @file CAN.c
 * @author 511844767
 * @brief CAN的静默回环工作测试
 * @version 0.1
 * @date 2023-01-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include"stdio.h"
#include"string.h"
#include"stm32f10x.h"
#include"stm32f10x_can.h"
#include"KEY.h"
#include"delay.h"

/* 32bit的ID号 */
#define ID1 ((uint32_t)0X1314)

/**
 * @brief CAN1对应PA11(RX)和PA12(TX)
 * 
 */
void CAN_Silent_LoopBack_Init(){
    /* 使能时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);    // F103只有CAN1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /* 初始化GPIO */
    GPIO_InitTypeDef gpioDef;
    gpioDef.GPIO_Speed  = GPIO_Speed_50MHz;
    gpioDef.GPIO_Pin    = GPIO_Pin_11;  // RX
    gpioDef.GPIO_Mode   = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpioDef);
    gpioDef.GPIO_Pin    = GPIO_Pin_12;  // TX
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpioDef);
    /* 初始化CAN */
    CAN_InitTypeDef canDef;
    canDef.CAN_ABOM     = ENABLE;                   // 使能自动离线退出(ENABLE: 只检测CAN协议的离线退出条件，DISABLE: 需要手动置位后再检测CAN协议的离线退出条件)
    canDef.CAN_AWUM     = ENABLE;                   // 使能硬件自动唤醒
    canDef.CAN_BS1      = CAN_BS1_5tq;              // BS1 = 5Tq
    canDef.CAN_BS2      = CAN_BS2_3tq;              // BS2 = 3Tq
    canDef.CAN_Mode     = CAN_Mode_Silent_LoopBack; // 静默回环工作模式
    canDef.CAN_NART     = ENABLE;                   // 使能自动重传(报文发送失败后尝试重新传输)
    canDef.CAN_Prescaler= 1;                        // Tq = 1 / 36000000 s
    canDef.CAN_RFLM     = ENABLE;                   // 当FIFO溢出时，是否锁定FIFO
    canDef.CAN_SJW      = CAN_SJW_4tq;              // 1bit信号长度允许吸收的误差大小
    canDef.CAN_TTCM     = DISABLE;                  // 时间触发通信（ISO11898-4标准，用不上)
    canDef.CAN_TXFP     = DISABLE;                  // FIFO发送顺序按照ID优先级发送
    CAN_Init(CAN1, &canDef);
    /* 初始化CAN过滤器 */
    CAN_FilterInitTypeDef canFilterDef;
    canFilterDef.CAN_FilterActivation       = ENABLE;               // 使能过滤器
    canFilterDef.CAN_FilterFIFOAssignment   = CAN_Filter_FIFO0;     // 给过滤器分配到FIFO邮箱
    /**
     * STM32 32bit的FilterID = 29bit的ID + 1bit的IDE + 1bit的RTR + 1bit的r0
     */
    canFilterDef.CAN_FilterIdHigh           = (((ID1 << 3) | CAN_ID_EXT | CAN_RTR_DATA) & 0XFFFF0000) >> 16; // 扩展数据帧高16bit
    canFilterDef.CAN_FilterIdLow            = (((ID1 << 3) | CAN_ID_EXT | CAN_RTR_DATA) & 0X0000FFFF) >> 0;  // 扩展数据帧低16bit
    canFilterDef.CAN_FilterMaskIdHigh       = 0XFFFF;               // 过滤掩码高16bit（1表示严格匹配，0表示任意匹配）
    canFilterDef.CAN_FilterMaskIdLow        = 0XFFFF;               // 过滤掩码低16bit
    canFilterDef.CAN_FilterMode             = CAN_FilterMode_IdMask;// 掩码工作模式（关键字匹配）
    canFilterDef.CAN_FilterNumber           = 0;                    // 第几个过滤器
    canFilterDef.CAN_FilterScale            = CAN_FilterScale_32bit;// 过滤器宽度为32bit
    CAN_FilterInit(&canFilterDef);
    /* 配置CAN中断 */
    NVIC_InitTypeDef nvicDef;
    nvicDef.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn; // USB_LP_CAN1_RX0_IRQn表示FIFO邮箱0，CAN1_RX1_IRQn表示FIFO邮箱1
    nvicDef.NVIC_IRQChannelCmd = ENABLE;
    nvicDef.NVIC_IRQChannelPreemptionPriority = 2;
    nvicDef.NVIC_IRQChannelSubPriority = 2;
    NVIC_Init(&nvicDef);
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);       // 使能FIFO邮箱0中断
}

/* CAN FIFO0中断服务函数 */
void USB_LP_CAN1_RX0_IRQHandler(){
    CanRxMsg RxMessage;
    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
    printf("Recieve: %#X, %#X, %#X, %#X, %#X, %#X, %#X, %#X\r\n", 
        RxMessage.Data[0], RxMessage.Data[1], 
        RxMessage.Data[2], RxMessage.Data[3], 
        RxMessage.Data[4], RxMessage.Data[5], 
        RxMessage.Data[6], RxMessage.Data[7]);
}

extern void USART1_Init();

void CAN_Silent_LoopBack_Run(){
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    delay_init();
    USART1_Init();
    KEY_Init();
    CAN_Silent_LoopBack_Init();

    CanTxMsg TxMessage;
    uint8_t buffer[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    uint8_t transmit_mailbox;

    printf("按下KEY0发送数据: \r\n");
    while(1){
        KEY_STATUS_t key = KEY_Scan(KEY_SCAN_ONCE);
        if(key != KEY_NONE_PRESS){
            switch (key)
            {
            case KEY0_PRESS:    // 发送1帧数据
                memcpy(TxMessage.Data, buffer, 8);
                TxMessage.DLC       = 8;                                // 数据长度
                TxMessage.ExtId     = ID1;                              // ID号（扩展帧）
                TxMessage.IDE       = CAN_Id_Extended;                  // 扩展帧标志
                TxMessage.RTR       = CAN_RTR_Data;                     // 数据帧标志
                TxMessage.StdId     = 0;                                // 标准帧ID号（没用上）
                transmit_mailbox    = CAN_Transmit(CAN1, &TxMessage);   // 发送并返回邮箱号
                while(CAN_TransmitStatus(CAN1, transmit_mailbox) != CAN_TxStatus_Ok);   // 等待邮箱发送完毕
                for(int i = 0; i < 8; ++i)
                    ++buffer[i];
                break;
            default:
                break;
            }
        }
    }
}