#include"stdio.h"
#include"stdbool.h"
#include"stm32f10x.h"
#include"stm32f10x_spi.h"
#include"delay.h"

/**
 * @brief 用SPI协议读写外部FLASH
 * @note  NM25Q128EVB重置后第一个command有问题，用Wake Up函数消耗它
 * TODO:测试一下不同SPI模式能不能读写flash
 * 
 */

#define MIN(a, b) (a < b)? a: b 

/* 片选信号控制 */
#define Flash_EX_CS_Low()   GPIO_ResetBits(GPIOB, GPIO_Pin_12)
#define Flash_EX_CS_High()  GPIO_SetBits(GPIOB, GPIO_Pin_12)

/* FLASH数据 */
#define FLASH_EX_SIZE 128*1024*1024     // flash内存大小

void Flash_EX_Init(){
    /* 使能时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    /* 配置GPIO引脚 */
    GPIO_InitTypeDef gpioDef;
    gpioDef.GPIO_Speed  = GPIO_Speed_50MHz;
    gpioDef.GPIO_Pin    = GPIO_Pin_13 | GPIO_Pin_15;         // SCK MOSI
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &gpioDef);
    gpioDef.GPIO_Pin    = GPIO_Pin_14;                       // MISO
    gpioDef.GPIO_Mode   = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &gpioDef);
    gpioDef.GPIO_Pin    = GPIO_Pin_12;                       // CS
    gpioDef.GPIO_Mode   = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &gpioDef);
    /* 配置SPI2 */
    SPI_InitTypeDef spiDef;
    spiDef.SPI_CPHA             = SPI_CPHA_2Edge;                   // SPI模式0
    spiDef.SPI_CPOL             = SPI_CPOL_High;                    // SPI模式0
    spiDef.SPI_CRCPolynomial    = 0X7;                              // CRC校验位（用不上）
    spiDef.SPI_DataSize         = SPI_DataSize_8b;                  // 数据大小
    spiDef.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;         // 二分频（PCLK1为36MHz，SPI2分频到18MHz）
    spiDef.SPI_Direction        = SPI_Direction_2Lines_FullDuplex;  // 全双工模式
    spiDef.SPI_FirstBit         = SPI_FirstBit_MSB;                 // MSB高位先发送模式（由FLASH芯片datasheet决定）
    spiDef.SPI_Mode             = SPI_Mode_Master;                  // 主机
    spiDef.SPI_NSS              = SPI_NSS_Soft;                     // 软件手动控制片选信号
    SPI_Init(SPI2, &spiDef);
    /* 拉高片选信号 */
    Flash_EX_CS_High();
    /* 使能SPI2 */
    SPI_Cmd(SPI2, ENABLE);
}

/* 异常检查 */
typedef enum Flash_EX_Errors{
    FLASH_EX_ERR_SUCCESS = 0, 
    FLASH_EX_ERR_TIMEOUT, 
    FLASH_EX_OVERFLOWED
}Flash_EX_Errors_t;

typedef struct Flash_EX_Errors_Info{
    const char info[20];
} Flash_EX_Errors_Info_t;

static Flash_EX_Errors_Info_t errInfo[3] = {
    {"success"}, 
    {"time out"}, 
    {"out of bounds"}
};

#define FLASH_EX_CHECK(func){ \
    err = func; \
    if(err != FLASH_EX_ERR_SUCCESS){ \
        fprintf(stderr, "Error %s, line: %d, function: %s\r\n", errInfo[err].info, __LINE__, __FUNCTION__); \
        goto FLASH_EX_ERR_FLAG; \
    } \
}

/* 等待状态 */
#define CHECK_FLAG(flag){ \
    uint16_t timeout = 200; \
    while(SPI_I2S_GetFlagStatus(SPI2, flag) == RESET){ \
        if(timeout-- == 0){ \
            err = FLASH_EX_ERR_TIMEOUT; \
            fprintf(stderr, "Flag error, line: %d, function: %s\r\n", __LINE__, __FUNCTION__); \
            goto FLASH_EX_ERR_FLAG; \
        } \
    } \
}

/* 越界检查 */
#define CHECK_BOUND(addr){ \
    if(addr >= FLASH_EX_SIZE){ \
        err = FLASH_EX_OVERFLOWED; \
        fprintf(stderr, "Error %s, line: %d, function: %s\r\n", errInfo[err].info, __LINE__, __FUNCTION__); \
        goto FLASH_EX_ERR_FLAG; \
    } \
}

#define DUMMY_BYTE 0XFF

Flash_EX_Errors_t __Flash_EX_Byte_Impl(uint8_t *out, uint8_t in){
    Flash_EX_Errors_t err = 0;
    CHECK_FLAG(SPI_I2S_FLAG_TXE);
    SPI_I2S_SendData(SPI2, (uint16_t)in);
    CHECK_FLAG(SPI_I2S_FLAG_RXNE);
    *out = (uint8_t)SPI_I2S_ReceiveData(SPI2);
FLASH_EX_ERR_FLAG:
    return err;
}

Flash_EX_Errors_t Flash_EX_Send_Byte(uint8_t byte){
    Flash_EX_Errors_t err = 0;
    uint8_t dummy = DUMMY_BYTE;
    FLASH_EX_CHECK(__Flash_EX_Byte_Impl(&dummy, byte));
FLASH_EX_ERR_FLAG:
    return err;
}

Flash_EX_Errors_t Flash_EX_Recieve_Byte(uint8_t* byte){
    Flash_EX_Errors_t err = 0;
    uint8_t dummy = DUMMY_BYTE;
    FLASH_EX_CHECK(__Flash_EX_Byte_Impl(byte, dummy));
FLASH_EX_ERR_FLAG:
    return err;
}

/* 唤醒设备 */
Flash_EX_Errors_t Flash_EX_Wake_Up(){
    Flash_EX_Errors_t err = 0;
    Flash_EX_CS_Low();
    FLASH_EX_CHECK(Flash_EX_Send_Byte(0XAB));
    FLASH_EX_CHECK(Flash_EX_Send_Byte(DUMMY_BYTE));
    FLASH_EX_CHECK(Flash_EX_Send_Byte(DUMMY_BYTE));
    FLASH_EX_CHECK(Flash_EX_Send_Byte(DUMMY_BYTE));
    uint8_t dummy = DUMMY_BYTE;
    FLASH_EX_CHECK(Flash_EX_Recieve_Byte(&dummy));  // 理论上这里可以返回ID，但实际上第一个command有问题，只返回0XFF
    Flash_EX_CS_High();
FLASH_EX_ERR_FLAG:
    return err;
}

/* 读取并检查 */
Flash_EX_Errors_t Flash_EX_Check_JEDEC_ID(){
    Flash_EX_Errors_t err = 0;
    Flash_EX_CS_Low();
    FLASH_EX_CHECK(Flash_EX_Send_Byte(0X9F));
    uint8_t idPart[3];
    FLASH_EX_CHECK(Flash_EX_Recieve_Byte(&idPart[0]));
    FLASH_EX_CHECK(Flash_EX_Recieve_Byte(&idPart[1]));
    FLASH_EX_CHECK(Flash_EX_Recieve_Byte(&idPart[2]));
    Flash_EX_CS_High();
    uint32_t id = ((uint32_t)idPart[0] << 16) | ((uint32_t)idPart[1] << 8) | ((uint32_t)idPart[2]);
    if(id == 0X522118){
        printf("JEDEC verify success!\r\n");
    }else{
        printf("JEDEC verify failed: id=%#x\r\n", (unsigned int)id);
    }
FLASH_EX_ERR_FLAG:
    Flash_EX_CS_High();
    return err;
}

/* 写使能 */
Flash_EX_Errors_t Flash_EX_Write_Enable(){
    Flash_EX_Errors_t err = 0;
    Flash_EX_CS_Low();
    FLASH_EX_CHECK(Flash_EX_Send_Byte(0X06));
FLASH_EX_ERR_FLAG:
    Flash_EX_CS_High();
    return err;
}

/* 写关闭 */
Flash_EX_Errors_t Flash_EX_Write_Disable(){
    Flash_EX_Errors_t err = 0;
    Flash_EX_CS_Low();
    FLASH_EX_CHECK(Flash_EX_Send_Byte(0X04));
FLASH_EX_ERR_FLAG:
    Flash_EX_CS_High();
    return err;
}

/* 读取状态寄存器 */
Flash_EX_Errors_t Flash_EX_Read_SR(uint8_t* reg){
    Flash_EX_Errors_t err = 0;
    Flash_EX_CS_Low();
    FLASH_EX_CHECK(Flash_EX_Send_Byte(0X05));
    FLASH_EX_CHECK(Flash_EX_Recieve_Byte(reg));
FLASH_EX_ERR_FLAG:
    Flash_EX_CS_High();
    return err;
}

/* 等待BUSY状态 */
Flash_EX_Errors_t Flash_EX_Synchronize(){
    uint8_t buffer;
    while(1){
        Flash_EX_Read_SR(&buffer);
        if(buffer & 0X01) delay_us(5);
        else break;
    }  // 检测BUSY标志
    return 0;
}

/* 擦除1Sector(4K) */
Flash_EX_Errors_t Flash_EX_Erase_Sector(uint32_t addr){
    Flash_EX_Errors_t err = 0;
    /* 判断是否越界 */
    CHECK_BOUND(addr);

    /* 擦除 */
    FLASH_EX_CHECK(Flash_EX_Synchronize(10000));
    FLASH_EX_CHECK(Flash_EX_Write_Enable());
    Flash_EX_CS_Low();
    FLASH_EX_CHECK(Flash_EX_Send_Byte(0X20));   // Erase Sector
    FLASH_EX_CHECK(Flash_EX_Send_Byte((addr & 0XFF0000) >> 16));
    FLASH_EX_CHECK(Flash_EX_Send_Byte((addr & 0X00FF00) >>  8));
    FLASH_EX_CHECK(Flash_EX_Send_Byte((addr & 0X0000FF) >>  0));
    Flash_EX_CS_High();
    FLASH_EX_CHECK(Flash_EX_Synchronize());
    FLASH_EX_CHECK(Flash_EX_Write_Disable());
FLASH_EX_ERR_FLAG:
    Flash_EX_CS_High();
    return err;
}

/* 读取数据 */
Flash_EX_Errors_t Flash_EX_Read(uint8_t* dst, uint32_t addr, uint32_t size){
    Flash_EX_Errors_t err = 0;
    /* 判断是否越界 */
    CHECK_BOUND(addr + size - 1);

    /* 读取 */
    FLASH_EX_CHECK(Flash_EX_Synchronize());
    Flash_EX_CS_Low();
    FLASH_EX_CHECK(Flash_EX_Send_Byte(0X0B));   // Fast Read
    FLASH_EX_CHECK(Flash_EX_Send_Byte((addr & 0XFF0000) >> 16));
    FLASH_EX_CHECK(Flash_EX_Send_Byte((addr & 0X00FF00) >>  8));
    FLASH_EX_CHECK(Flash_EX_Send_Byte((addr & 0X0000FF) >>  0));
    FLASH_EX_CHECK(Flash_EX_Send_Byte(DUMMY_BYTE));
    uint32_t idx;
    for(idx = 0; idx < size; ++idx){
        FLASH_EX_CHECK(Flash_EX_Recieve_Byte(&dst[idx]));
    }
    Flash_EX_CS_High();
    FLASH_EX_CHECK(Flash_EX_Synchronize());
FLASH_EX_ERR_FLAG:
    Flash_EX_CS_High();
    return err;
}

/* 判断区域是否为空 */
Flash_EX_Errors_t Flash_EX_Is_Empty(bool* flag, uint32_t addr, uint32_t size){
    Flash_EX_Errors_t err = 0;
    /* 判断是否越界 */
    CHECK_BOUND(addr + size - 1);

    /* 读取 */
    *flag = true;
    FLASH_EX_CHECK(Flash_EX_Synchronize());
    Flash_EX_CS_Low();
    FLASH_EX_CHECK(Flash_EX_Send_Byte(0X0B));   // Fast Read
    FLASH_EX_CHECK(Flash_EX_Send_Byte((addr & 0XFF0000) >> 16));
    FLASH_EX_CHECK(Flash_EX_Send_Byte((addr & 0X00FF00) >>  8));
    FLASH_EX_CHECK(Flash_EX_Send_Byte((addr & 0X0000FF) >>  0));
    FLASH_EX_CHECK(Flash_EX_Send_Byte(DUMMY_BYTE));
    uint32_t idx;
    uint8_t buffer;
    for(idx = 0; idx < size; ++idx){
        FLASH_EX_CHECK(Flash_EX_Recieve_Byte(&buffer));
        if(buffer != 0XFF){
            *flag = false;
            break;
        }
    }
    Flash_EX_CS_High();
    FLASH_EX_CHECK(Flash_EX_Synchronize());
FLASH_EX_ERR_FLAG:
    Flash_EX_CS_High();
    return err;
}

/* 擦除非空区域 */
Flash_EX_Errors_t Flash_EX_Erase_NonEmpty(uint32_t addr, uint32_t size){
    Flash_EX_Errors_t err = 0;
    /* 判断是否越界 */
    CHECK_BOUND(addr + size - 1);
    
    /* 先擦除到sector对齐 */
    bool flag;
    uint32_t count = MIN(4096 - addr % 4096, size);
    uint32_t idx;
    FLASH_EX_CHECK(Flash_EX_Is_Empty(&flag, addr, count));
    if(!flag) 
        FLASH_EX_CHECK(Flash_EX_Erase_Sector(addr));
    addr += count;
    size -= count;
    /* 再把剩下的数据擦除 */
    uint32_t sectors = (size + 4095) / 4096;
    for(idx = 0; idx < sectors; ++idx){
        count = MIN(size, 4096);
        FLASH_EX_CHECK(Flash_EX_Is_Empty(&flag, addr, count));
        if(!flag) 
            FLASH_EX_CHECK(Flash_EX_Erase_Sector(addr));
        addr += count;
        size -= count;
    }
FLASH_EX_ERR_FLAG:
    Flash_EX_CS_High();
    return err;
}

/* 写入数据 */
Flash_EX_Errors_t Flash_EX_Write(uint8_t* src, uint32_t addr, uint32_t size){
    Flash_EX_Errors_t err = 0;
    /* 判断是否越界 */
    CHECK_BOUND(addr + size - 1);

    /* 擦除非空区域 */
    FLASH_EX_CHECK(Flash_EX_Erase_NonEmpty(addr, size));

    /* 写使能 */
    FLASH_EX_CHECK(Flash_EX_Write_Enable());

    /* 先写到页对齐 */
    uint32_t count = MIN(256 - addr % 256, size);
    uint32_t idx;
    FLASH_EX_CHECK(Flash_EX_Synchronize());
    Flash_EX_CS_Low();
    FLASH_EX_CHECK(Flash_EX_Send_Byte(0X02));   // Page Program
    FLASH_EX_CHECK(Flash_EX_Send_Byte((addr & 0XFF0000) >> 16));
    FLASH_EX_CHECK(Flash_EX_Send_Byte((addr & 0X00FF00) >>  8));
    FLASH_EX_CHECK(Flash_EX_Send_Byte((addr & 0X0000FF) >>  0));
    for(idx = 0; idx < count; ++idx){
        FLASH_EX_CHECK(Flash_EX_Send_Byte(src[idx]));
    }
    Flash_EX_CS_High();
    FLASH_EX_CHECK(Flash_EX_Synchronize());
    src += count;
    size -= count;
    addr += count;

    /* 再把剩下的数据写完 */
    uint32_t pages = (size + 255) / 256;
    for(uint32_t p = 0; p < pages; ++p){
        FLASH_EX_CHECK(Flash_EX_Synchronize());
        Flash_EX_CS_Low();
        FLASH_EX_CHECK(Flash_EX_Send_Byte(0X02));   // Page Program
        FLASH_EX_CHECK(Flash_EX_Send_Byte((addr & 0XFF0000) >> 16));
        FLASH_EX_CHECK(Flash_EX_Send_Byte((addr & 0X00FF00) >>  8));
        FLASH_EX_CHECK(Flash_EX_Send_Byte((addr & 0X0000FF) >>  0));
        for(uint8_t i = 0; i < 256; ++i){
            idx = i + p * 256;
            if(idx >= size) 
                break;
            FLASH_EX_CHECK(Flash_EX_Send_Byte(src[idx]));
        }
        addr += 256;
        Flash_EX_CS_High();
    }
    FLASH_EX_CHECK(Flash_EX_Synchronize());

    /* 写失能 */
    FLASH_EX_CHECK(Flash_EX_Write_Disable());
FLASH_EX_ERR_FLAG:
    Flash_EX_CS_High();
    return err;
}

extern void USART1_Init();

void Flash_EX_Run(){
    Flash_EX_Errors_t err = 0;

    /* 初始化 */
    delay_init();
    USART1_Init();  
    Flash_EX_Init();

    /* 唤醒设备 */
    FLASH_EX_CHECK(Flash_EX_Wake_Up());

    /* 验证JEDEC设备ID */
    FLASH_EX_CHECK(Flash_EX_Check_JEDEC_ID());

    /* 擦除扇区 */
    uint32_t addr = FLASH_EX_SIZE - 100;
    FLASH_EX_CHECK(Flash_EX_Erase_Sector(addr));    // 擦除最后一个扇区
    printf("Erase sector success!\r\n");

    /* 写数据 */
    const char src[100] = "Ava Diana";
    FLASH_EX_CHECK(Flash_EX_Write((uint8_t*)src, addr, 100));

    /* 读取数据 */
    char buffer[100];
    FLASH_EX_CHECK(Flash_EX_Read((uint8_t*)buffer, addr, 100));
    printf("Read Data: %s\r\n", buffer);

FLASH_EX_ERR_FLAG:
    return;
}