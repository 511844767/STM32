#ifndef GPIO_SPI_H__
#define GPIO_SPI_H__
#include"stm32f10x.h"

/**
 * @brief 初始化引脚
 * 
 */
void GPIO_SPI_Init();

/**
 * @brief 发送数据
 * @note 上升沿采样，高位先行
 * 
 * @param src 
 * @param size 
 */
void GPIO_SPI_SendData(void* src, uint8_t size);

/**
 * @brief 接收数据
 * @note 上升沿后立即采样，高位先行
 * 
 * @param dst 
 * @param size 
 */
void GPIO_SPI_RecieveData(void* dst, uint8_t size);

#endif
