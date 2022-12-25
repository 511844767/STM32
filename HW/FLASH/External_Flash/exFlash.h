#ifndef FLASH_EX_H__
#define FLASH_EX_H__
#include"stdbool.h"
#include"delay.h"

extern void USART1_Init();

/* 异常检查 */
typedef enum Flash_EX_Errors{
    FLASH_EX_ERR_SUCCESS = 0, 
    FLASH_EX_ERR_TIMEOUT, 
    FLASH_EX_OVERFLOWED
}Flash_EX_Errors_t;

/* 读写接口 */
void Flash_EX_Init();
Flash_EX_Errors_t Flash_EX_Wake_Up();
Flash_EX_Errors_t Flash_EX_Check_JEDEC_ID(bool* flag);
Flash_EX_Errors_t Flash_EX_Read(uint8_t* dst, uint32_t addr, uint32_t size);
Flash_EX_Errors_t Flash_EX_Write(uint8_t* src, uint32_t addr, uint32_t size);

#endif