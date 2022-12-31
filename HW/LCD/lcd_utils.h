#ifndef LCD_UTILS_H__
#define LCD_UTILS_H__
#include"stdio.h"
#include"stdbool.h"
#include"stm32f10x.h"
#include"stm32f10x_fsmc.h"
#include"delay.h"

/* 引脚  */
// CS
#define LCD_GPIO_CS GPIOG
#define LCD_PIN_CS  GPIO_Pin_12
// RS
#define LCD_GPIO_RS GPIOG
#define LCD_PIN_RS  GPIO_Pin_0
// WR
#define LCD_GPIO_WR GPIOD
#define LCD_PIN_WR  GPIO_Pin_5
// RD
#define LCD_GPIO_RD GPIOD
#define LCD_PIN_RD  GPIO_Pin_4
// D0
#define LCD_GPIO_D0 GPIOD
#define LCD_PIN_D0  GPIO_Pin_14
// D1
#define LCD_GPIO_D1 GPIOD
#define LCD_PIN_D1  GPIO_Pin_15
// D2
#define LCD_GPIO_D2 GPIOD
#define LCD_PIN_D2  GPIO_Pin_0
// D3
#define LCD_GPIO_D3 GPIOD
#define LCD_PIN_D3  GPIO_Pin_1
// D4
#define LCD_GPIO_D4 GPIOE
#define LCD_PIN_D4  GPIO_Pin_7
// D5
#define LCD_GPIO_D5 GPIOE
#define LCD_PIN_D5  GPIO_Pin_8
// D6
#define LCD_GPIO_D6 GPIOE
#define LCD_PIN_D6  GPIO_Pin_9
// D7
#define LCD_GPIO_D7 GPIOE
#define LCD_PIN_D7  GPIO_Pin_10
// D8
#define LCD_GPIO_D8 GPIOE
#define LCD_PIN_D8  GPIO_Pin_11
// D9
#define LCD_GPIO_D9 GPIOE
#define LCD_PIN_D9  GPIO_Pin_12
// D10
#define LCD_GPIO_D10 GPIOE
#define LCD_PIN_D10  GPIO_Pin_13
// D11
#define LCD_GPIO_D11 GPIOE
#define LCD_PIN_D11  GPIO_Pin_14
// D12
#define LCD_GPIO_D12 GPIOE
#define LCD_PIN_D12  GPIO_Pin_15
// D13
#define LCD_GPIO_D13 GPIOD
#define LCD_PIN_D13  GPIO_Pin_8
// D14
#define LCD_GPIO_D14 GPIOD
#define LCD_PIN_D14  GPIO_Pin_9
// D15
#define LCD_GPIO_D15 GPIOD
#define LCD_PIN_D15  GPIO_Pin_10
// BL
#define LCD_GPIO_BL GPIOB
#define LCD_PIN_BL  GPIO_Pin_0
// MI
#define LCD_GPIO_MI GPIOB
#define LCD_PIN_MI  GPIO_Pin_2
// CLK
#define LCD_GPIO_CLK GPIOB
#define LCD_PIN_CLK  GPIO_Pin_1
// MO
#define LCD_GPIO_MO GPIOF
#define LCD_PIN_MO  GPIO_Pin_9
// PEN
#define LCD_GPIO_PEN GPIOF
#define LCD_PIN_PEN  GPIO_Pin_10
// TCS
#define LCD_GPIO_TCS GPIOF
#define LCD_PIN_TCS  GPIO_Pin_11

/* 初始化GPIO */
#define LCD_CONFIG_GPIO(){ \
    GPIO_InitTypeDef gpioDef; \
    gpioDef.GPIO_Speed  = GPIO_Speed_50MHz;  \
    gpioDef.GPIO_Pin    = LCD_PIN_BL;       /* BL背光控制（非FSMC \引脚，普通推挽输出）*/ \
    gpioDef.GPIO_Mode   = GPIO_Mode_Out_PP;  \
    GPIO_Init(LCD_GPIO_BL, &gpioDef);  \
    gpioDef.GPIO_Pin    = LCD_PIN_CS;       /* CS->FSMC_NE4 */  \
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP;  \
    GPIO_Init(LCD_GPIO_CS, &gpioDef);  \
    gpioDef.GPIO_Pin    = LCD_PIN_RS;       /* RS->FSMC_A10  */ \
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP;  \
    GPIO_Init(LCD_GPIO_RS, &gpioDef);  \
    gpioDef.GPIO_Pin    = LCD_PIN_WR;       /* WR->FSMC_NWE  */ \
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP;  \
    GPIO_Init(LCD_GPIO_WR, &gpioDef);  \
    gpioDef.GPIO_Pin    = LCD_PIN_RD;       /* RD->FSMC_NOE */ \
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP;  \
    GPIO_Init(LCD_GPIO_RD, &gpioDef);  \
    gpioDef.GPIO_Pin    = LCD_PIN_D0;       /* D0~15->FSMC_D0~15 */ \
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP;  \
    GPIO_Init(LCD_GPIO_D0, &gpioDef); \
    gpioDef.GPIO_Pin    = LCD_PIN_D1; \
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP; \
    GPIO_Init(LCD_GPIO_D1, &gpioDef); \
    gpioDef.GPIO_Pin    = LCD_PIN_D2; \
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP; \
    GPIO_Init(LCD_GPIO_D2, &gpioDef); \
    gpioDef.GPIO_Pin    = LCD_PIN_D3; \
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP; \
    GPIO_Init(LCD_GPIO_D3, &gpioDef); \
    gpioDef.GPIO_Pin    = LCD_PIN_D4; \
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP; \
    GPIO_Init(LCD_GPIO_D4, &gpioDef); \
    gpioDef.GPIO_Pin    = LCD_PIN_D5; \
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP; \
    GPIO_Init(LCD_GPIO_D5, &gpioDef); \
    gpioDef.GPIO_Pin    = LCD_PIN_D6; \
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP; \
    GPIO_Init(LCD_GPIO_D6, &gpioDef); \
    gpioDef.GPIO_Pin    = LCD_PIN_D7; \
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP; \
    GPIO_Init(LCD_GPIO_D7, &gpioDef); \
    gpioDef.GPIO_Pin    = LCD_PIN_D8; \
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP; \
    GPIO_Init(LCD_GPIO_D8, &gpioDef); \
    gpioDef.GPIO_Pin    = LCD_PIN_D9; \
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP; \
    GPIO_Init(LCD_GPIO_D9, &gpioDef); \
    gpioDef.GPIO_Pin    = LCD_PIN_D10; \
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP; \
    GPIO_Init(LCD_GPIO_D10, &gpioDef); \
    gpioDef.GPIO_Pin    = LCD_PIN_D11; \
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP; \
    GPIO_Init(LCD_GPIO_D11, &gpioDef); \
    gpioDef.GPIO_Pin    = LCD_PIN_D12; \
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP; \
    GPIO_Init(LCD_GPIO_D12, &gpioDef); \
    gpioDef.GPIO_Pin    = LCD_PIN_D13; \
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP; \
    GPIO_Init(LCD_GPIO_D13, &gpioDef); \
    gpioDef.GPIO_Pin    = LCD_PIN_D14; \
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP; \
    GPIO_Init(LCD_GPIO_D14, &gpioDef); \
    gpioDef.GPIO_Pin    = LCD_PIN_D15; \
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP; \
    GPIO_Init(LCD_GPIO_D15, &gpioDef); \
}

/**
 * NT35310 LCD采用8080时序的异步通信，分辨率为320*480
 * FSMC时钟频率为72MHz，1个时钟周期为1 / (72 * 1024 * 1024) * 1000000000 ≈ 13.245ns
 * FSMC_MODEA: 查表可知
 *    写取ADDSET = 0, DATASET = 2, t_wrh = 2满足ILI9341时序要求
 *    读取ADDSET = 0, DATASET = 24, t_rdh/t_rdhfm = 7满足ILI9341时序要求（其中t_rodh的实际数值没有任何模式能给出，可能有风险）
 * 结构体文档参考：https://blog.csdn.net/weixin_43871650/article/details/105579620
 */
/* 初始化FSMC */
#define LCD_CONFIG_FSMC(){ \
    FSMC_NORSRAMTimingInitTypeDef fsmcNSWriteTimeDef;   /* 写时序 */ \
    fsmcNSWriteTimeDef.FSMC_AccessMode              = FSMC_AccessMode_A;    /* 模式A */ \
    fsmcNSWriteTimeDef.FSMC_AddressHoldTime         = 0;                    /* 模式D的ADDHLD参数（用不上）*/ \
    fsmcNSWriteTimeDef.FSMC_AddressSetupTime        = 0;                    /* ADDSET设置为0 */ \
    fsmcNSWriteTimeDef.FSMC_BusTurnAroundDuration   = 0;                    /* 总线复用模式的延迟参数（用不上）*/ \
    fsmcNSWriteTimeDef.FSMC_CLKDivision             = 0;                    /* 时钟分频，不分频设置为0（72MHz）*/ \
    fsmcNSWriteTimeDef.FSMC_DataLatency             = 0;                    /* 同步通信的延时参数（用不上）*/ \
    fsmcNSWriteTimeDef.FSMC_DataSetupTime           = 2;                    /* DATASET设置为2 */ \
    FSMC_NORSRAMTimingInitTypeDef fsmcNSReadTimeDef;   /* 读时序 */ \
    fsmcNSReadTimeDef.FSMC_AccessMode               = FSMC_AccessMode_A;    /* 模式A */ \
    fsmcNSReadTimeDef.FSMC_AddressHoldTime          = 0;                    /* 模式D的ADDHLD参数（用不上）*/ \
    fsmcNSReadTimeDef.FSMC_AddressSetupTime         = 0;                    /* ADDSET设置为 0*/ \
    fsmcNSReadTimeDef.FSMC_BusTurnAroundDuration    = 0;                    /* 总线复用模式的延迟参数（用不上）*/ \
    fsmcNSReadTimeDef.FSMC_CLKDivision              = 0;                    /* 时钟分频，不分频设置为0（72MHz）*/ \
    fsmcNSReadTimeDef.FSMC_DataLatency              = 0;                    /* 同步通信的延时参数（用不上）*/ \
    fsmcNSReadTimeDef.FSMC_DataSetupTime            = 24;                   /* DATASET设置为24 */ \
    FSMC_NORSRAMInitTypeDef fsmcNSDef; \
    fsmcNSDef.FSMC_AsynchronousWait                 = FSMC_AsynchronousWait_Disable;        /* 同步通信参数 */ \
    fsmcNSDef.FSMC_Bank                             = FSMC_Bank1_NORSRAM4;                  /* FSMC_NE4连接了LCD片选信号线 */ \
    fsmcNSDef.FSMC_BurstAccessMode                  = FSMC_BurstAccessMode_Disable;         /* 同步通信参数 */ \
    fsmcNSDef.FSMC_DataAddressMux                   = FSMC_DataAddressMux_Disable;          /* 不复用地址、数据信号线（总线复用） */ \
    fsmcNSDef.FSMC_ExtendedMode                     = FSMC_ExtendedMode_Enable;             /* 扩展模式：读写时序是否分开配置（ENABLE: 模式A, DISABLE: 模式1） */ \
    fsmcNSDef.FSMC_MemoryDataWidth                  = FSMC_MemoryDataWidth_16b;             /* 数据宽度 */ \
    fsmcNSDef.FSMC_MemoryType                       = FSMC_MemoryType_NOR;                  /* LCD属于NOR存储器 */ \
    fsmcNSDef.FSMC_ReadWriteTimingStruct            = &fsmcNSReadTimeDef;                   /* 读时序 */ \
    fsmcNSDef.FSMC_WaitSignal                       = FSMC_WaitSignal_Disable;              /* 同步通信参数 */ \
    fsmcNSDef.FSMC_WaitSignalActive                 = FSMC_WaitSignalActive_BeforeWaitState;/* 同步通信参数 */ \
    fsmcNSDef.FSMC_WaitSignalPolarity               = FSMC_WaitSignalPolarity_Low;          /* 同步通信参数 */ \
    fsmcNSDef.FSMC_WrapMode                         = FSMC_WrapMode_Disable;                /* 同步通信参数 */ \
    fsmcNSDef.FSMC_WriteBurst                       = FSMC_WriteBurst_Disable;              /* 同步通信参数 */ \
    fsmcNSDef.FSMC_WriteOperation                   = FSMC_WriteOperation_Enable;           /* 存储器是否可写 */ \
    fsmcNSDef.FSMC_WriteTimingStruct                = &fsmcNSWriteTimeDef;                  /* 写时序 */ \
    FSMC_NORSRAMInit(&fsmcNSDef); \
}

/* 分辨率 */
#define LCD_WIDTH   320
#define LCD_HEIGHT  480

/**
 * 地址: FSMC_A10连接RS数据/命令切换信号
 *  NOR NE4的HADDR基地址为0X6000 0000 + 3 * 64 * 1024 * 1024 = 0X6C00 0000
 *  FSMC_A10 = 1时，HADDR地址为0X6C00 0000 + (1 << (10 + 1)) = 0X6C00 0800
 *  FSMC_A10 = 0时，HADDR地址为0X6C00 0000
 */
#define __LCD_ADDR_CMD          ((uint32_t)0X6C000000)
#define __LCD_ADDR_DATA         ((uint32_t)0X6C000800)
#define LCD_Write_Data(data)    *(__IO uint16_t*)__LCD_ADDR_DATA = data
#define LCD_Write_Cmd(cmd)      *(__IO uint16_t*)__LCD_ADDR_CMD = cmd
#define LCD_Read_Data()         (*(__IO uint16_t*)__LCD_ADDR_DATA)
static uint16_t dummy = 0XFF;
#define LCD_Read_Dummy()        { dummy = LCD_Read_Data(); }

/* 背光控制 */
#define LCD_BL_ON()     GPIO_SetBits(LCD_GPIO_BL, LCD_PIN_BL)
#define LCD_BL_OFF()    GPIO_ResetBits(LCD_GPIO_BL, LCD_PIN_BL)

/**
 * @brief 初始化序列 
 * TODO: 论坛说这个序列由厂家提供，但我没找到在哪查
 */
#define LCD_INIT_SEQUENCE(){ \
    LCD_Write_Cmd(0xED); \
    LCD_Write_Data(0x01); \
    LCD_Write_Data(0xFE); \
 \
    LCD_Write_Cmd(0xEE); \
    LCD_Write_Data(0xDE); \
    LCD_Write_Data(0x21); \
 \
    LCD_Write_Cmd(0xF1); \
    LCD_Write_Data(0x01); \
    LCD_Write_Cmd(0xDF); \
    LCD_Write_Data(0x10); \
 \
    /* VCOMvoltage */ \
    LCD_Write_Cmd(0xC4); \
    LCD_Write_Data(0x8F);      /* 5f */ \
 \
    LCD_Write_Cmd(0xC6); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xE2); \
    LCD_Write_Data(0xE2); \
    LCD_Write_Data(0xE2); \
    LCD_Write_Cmd(0xBF); \
    LCD_Write_Data(0xAA); \
 \
    LCD_Write_Cmd(0xB0); \
    LCD_Write_Data(0x0D); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x0D); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x11); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x19); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x21); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x2D); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x3D); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x5D); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x5D); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xB1); \
    LCD_Write_Data(0x80); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x8B); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x96); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xB2); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x02); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x03); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xB3); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xB4); \
    LCD_Write_Data(0x8B); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x96); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xA1); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xB5); \
    LCD_Write_Data(0x02); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x03); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x04); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xB6); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xB7); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x3F); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x5E); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x64); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x8C); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xAC); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xDC); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x70); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x90); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xEB); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xDC); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xB8); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xBA); \
    LCD_Write_Data(0x24); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xC1); \
    LCD_Write_Data(0x20); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x54); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xFF); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xC2); \
    LCD_Write_Data(0x0A); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x04); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xC3); \
    LCD_Write_Data(0x3C); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x3A); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x39); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x37); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x3C); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x36); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x32); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x2F); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x2C); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x29); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x26); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x24); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x24); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x23); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x3C); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x36); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x32); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x2F); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x2C); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x29); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x26); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x24); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x24); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x23); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xC4); \
    LCD_Write_Data(0x62); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x05); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x84); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xF0); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x18); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xA4); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x18); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x50); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x0C); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x17); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x95); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xF3); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xE6); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xC5); \
    LCD_Write_Data(0x32); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x44); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x65); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x76); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x88); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xC6); \
    LCD_Write_Data(0x20); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x17); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x01); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xC7); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xC8); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xC9); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xE0); \
    LCD_Write_Data(0x16); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x1C); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x21); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x36); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x46); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x52); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x64); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x7A); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x8B); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x99); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xA8); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xB9); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xC4); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xCA); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xD2); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xD9); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xE0); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xF3); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xE1); \
    LCD_Write_Data(0x16); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x1C); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x22); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x36); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x45); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x52); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x64); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x7A); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x8B); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x99); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xA8); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xB9); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xC4); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xCA); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xD2); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xD8); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xE0); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xF3); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xE2); \
    LCD_Write_Data(0x05); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x0B); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x1B); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x34); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x44); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x4F); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x61); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x79); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x88); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x97); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xA6); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xB7); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xC2); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xC7); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xD1); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xD6); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xDD); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xF3); \
    LCD_Write_Data(0x00); \
    LCD_Write_Cmd(0xE3); \
    LCD_Write_Data(0x05); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xA); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x1C); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x33); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x44); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x50); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x62); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x78); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x88); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x97); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xA6); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xB7); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xC2); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xC7); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xD1); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xD5); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xDD); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xF3); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xE4); \
    LCD_Write_Data(0x01); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x01); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x02); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x2A); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x3C); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x4B); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x5D); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x74); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x84); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x93); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xA2); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xB3); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xBE); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xC4); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xCD); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xD3); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xDD); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xF3); \
    LCD_Write_Data(0x00); \
    LCD_Write_Cmd(0xE5); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x02); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x29); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x3C); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x4B); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x5D); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x74); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x84); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x93); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xA2); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xB3); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xBE); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xC4); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xCD); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xD3); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xDC); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xF3); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xE6); \
    LCD_Write_Data(0x11); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x34); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x56); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x76); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x77); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x66); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x88); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x99); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xBB); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x99); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x66); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x55); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x55); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x45); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x43); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x44); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xE7); \
    LCD_Write_Data(0x32); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x55); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x76); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x66); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x67); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x67); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x87); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x99); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xBB); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x99); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x77); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x44); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x56); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x23); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x33); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x45); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xE8); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x99); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x87); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x88); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x77); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x66); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x88); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xAA); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0xBB); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x99); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x66); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x55); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x55); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x44); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x44); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x55); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xE9); \
    LCD_Write_Data(0xAA); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0x00); \
    LCD_Write_Data(0xAA); \
 \
    LCD_Write_Cmd(0xCF); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xF0); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x50); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xF3); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0xF9); \
    LCD_Write_Data(0x06); \
    LCD_Write_Data(0x10); \
    LCD_Write_Data(0x29); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0x3A); \
    LCD_Write_Data(0x55);	/* 66 */ \
 \
    LCD_Write_Cmd(0x11); \
    delay_ms(100); \
    LCD_Write_Cmd(0x29); \
    LCD_Write_Cmd(0x35); \
    LCD_Write_Data(0x00); \
 \
    LCD_Write_Cmd(0x51); \
    LCD_Write_Data(0xFF); \
    LCD_Write_Cmd(0x53); \
    LCD_Write_Data(0x2C); \
    LCD_Write_Cmd(0x55); \
    LCD_Write_Data(0x82); \
    LCD_Write_Cmd(0x2c); \
}

/* 矩形 */
typedef struct{
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
} LCD_Rectangle_t;

/* 直线 */
typedef struct{
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
} LCD_Line_t;

/* 颜色通道 */
typedef uint16_t LCD_Color_t;
#define LCD_Color_RGB565(R, G, B) ((LCD_Color_t)(((R & 0X1F) << 11) | ((G & 0X3F) << 5) | ((B & 0X1F) << 0)))

/* 计算式 */
#define LCD_MIN(a, b) ((a <= b)? a: b)
#define LCD_MAX(a, b) ((a >= b)? a: b)
static uint16_t __LCD_Temp;
#define LCD_SWAP(a, b) { __LCD_Temp = a; a = b; b = __LCD_Temp; }
#endif