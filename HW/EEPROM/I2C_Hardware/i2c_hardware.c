#include"stdio.h"
#include"stm32f10x.h"
#include"stm32f10x_i2c.h"
#include"delay.h"

/**
 * @brief 用STM32的I2C引脚硬件实现I2C通讯
 *          TODO: 异常检测太丑陋了，CHECK_FLAG也不能检测读写是否复位，还是需要用手动延迟
 */

void I2C_Hardware_Init(){
    /* 使能时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    /* 配置GPIO */
    GPIO_InitTypeDef gpioDef;
    gpioDef.GPIO_Mode   = GPIO_Mode_AF_OD;
    gpioDef.GPIO_Pin    = GPIO_Pin_6 | GPIO_Pin_7;
    gpioDef.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpioDef);
    /* 配置I2C1 */
    I2C_InitTypeDef iicDef;
    iicDef.I2C_ClockSpeed           = 400000;                       // 时钟为400KHz，周期为2.5us，满足EEPROM的高低电平有效时间要求
    iicDef.I2C_Mode                 = I2C_Mode_I2C;                 // I2C通信或者SMBus通信
    iicDef.I2C_DutyCycle            = I2C_DutyCycle_2;              // 占空比，似乎没什么用，Init函数会根据I2C_ClockSpeed自动配置I2C_DutyCycle
    iicDef.I2C_OwnAddress1          = 0X00A;                        // 本机设备地址1（STM32作为从机的时候地址才起作用）（STM32可以同时占用两个地址I2C_OwnAddress2）
    iicDef.I2C_Ack                  = I2C_Ack_Enable;               // 在接收到1byte数据后，是否自动产生应答信号
    iicDef.I2C_AcknowledgedAddress  = I2C_AcknowledgedAddress_7bit; // 设备地址位数
    I2C_Init(I2C1, &iicDef);
    /* 使能I2C1 */
    I2C_Cmd(I2C1, ENABLE);
}

/* 设备地址，0位根据读/写确定，1~3位从EEPROM电路图的A0~A2引脚确定，4~7位从Data Sheet查得 */
#define EEPROM_ADDRESS    0B10100000

/* 等待事件 */
static uint8_t eventTimeOut;
#define CHECK_EVENT(event) { \
    eventTimeOut = 200; \
    while(I2C_CheckEvent(I2C1, event) != SUCCESS){ \
        delay_us(5); \
        if(eventTimeOut-- == 0){ \
            fprintf(stderr, "Event error, line: %d, function: %s\r\n", __LINE__, __FUNCTION__); \
            break; \
        } \
    } \
}
#define CHECK_FLAG(flag) { \
    eventTimeOut = 200; \
    while(I2C_GetFlagStatus(I2C1, flag) != RESET){ \
        delay_us(5); \
        if(eventTimeOut-- == 0){ \
            fprintf(stderr, "Flag error, line: %d, function: %s\r\n", __LINE__, __FUNCTION__); \
            break; \
        } \
    } \
}

void Check_EEROM_IH(){
    CHECK_FLAG(I2C_FLAG_BUSY);

    I2C_GenerateSTART(I2C1, ENABLE);
    CHECK_EVENT(I2C_EVENT_MASTER_MODE_SELECT);
    I2C_Send7bitAddress(I2C1, EEPROM_ADDRESS, I2C_Direction_Transmitter);
    CHECK_EVENT(I2C_Direction_Transmitter);
    I2C_GenerateSTOP(I2C1, ENABLE);
}

void EEPROM_Write_Byte_IH(uint8_t addr, uint8_t data){
    /* 等待上一次的I2C信号结束 */
    CHECK_FLAG(I2C_FLAG_BUSY);

    /* 写入1Byte */
    I2C_GenerateSTART(I2C1, ENABLE);
    CHECK_EVENT(I2C_EVENT_MASTER_MODE_SELECT);
    I2C_Send7bitAddress(I2C1, EEPROM_ADDRESS, I2C_Direction_Transmitter);
    CHECK_EVENT(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
    I2C_SendData(I2C1, addr);
    CHECK_EVENT(I2C_EVENT_MASTER_BYTE_TRANSMITTING);
    I2C_SendData(I2C1, data);
    CHECK_EVENT(I2C_EVENT_MASTER_BYTE_TRANSMITTED);
    I2C_GenerateSTOP(I2C1, ENABLE);
}

uint8_t EEPROM_Read_Byte_IH(uint8_t addr){
    /* 等待上一次的I2C信号结束 */
    CHECK_FLAG(I2C_FLAG_BUSY);

    /* 读取1Byte */
    I2C_GenerateSTART(I2C1, ENABLE);
    CHECK_EVENT(I2C_EVENT_MASTER_MODE_SELECT);
    I2C_Send7bitAddress(I2C1, EEPROM_ADDRESS, I2C_Direction_Transmitter);
    CHECK_EVENT(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
    I2C_SendData(I2C1, addr);
    CHECK_EVENT(I2C_EVENT_MASTER_BYTE_TRANSMITTED);
    I2C_GenerateSTART(I2C1, ENABLE);
    CHECK_EVENT(I2C_EVENT_MASTER_MODE_SELECT);
    I2C_Send7bitAddress(I2C1, EEPROM_ADDRESS, I2C_Direction_Receiver);
    CHECK_EVENT(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);
    CHECK_EVENT(I2C_EVENT_MASTER_BYTE_RECEIVED);    // 注意一定要先等待EV7再读取DR寄存器，否则读取错误数据
    I2C_AcknowledgeConfig(I2C1, DISABLE);           // 最后一个receive信号之前关闭ACK信号
    uint8_t data = I2C_ReceiveData(I2C1);
    I2C_GenerateSTOP(I2C1, ENABLE);

    /* 等待stop信号完成后重新使能ACK信号 */
    CHECK_FLAG(I2C_FLAG_BUSY);
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    return data;
}

void EEPROM_Write_Sequential_IH(uint8_t addr, uint8_t* dataPtr, uint8_t dataLen){
    if((uint16_t)addr + (uint16_t)dataLen > 0XFF)
        fprintf(stderr, "Write bytes overflowed, line: %d, function: %s\r\n", __LINE__, __FUNCTION__);
    /* 写到addr对齐page */
#define MIN(a, b) (a < b)? a: b 
    uint8_t count = MIN(8 - addr % 8, dataLen);
    uint8_t idx;
    /* 等待上一次的I2C信号结束 */
    CHECK_FLAG(I2C_FLAG_BUSY);

    /* 写到addr对齐page */
    I2C_GenerateSTART(I2C1, ENABLE);
    CHECK_EVENT(I2C_EVENT_MASTER_MODE_SELECT);
    I2C_Send7bitAddress(I2C1, EEPROM_ADDRESS, I2C_Direction_Transmitter);
    CHECK_EVENT(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
    I2C_SendData(I2C1, addr);
    CHECK_EVENT(I2C_EVENT_MASTER_BYTE_TRANSMITTING);
    for(idx = 0; idx < count; ++idx){
        I2C_SendData(I2C1, dataPtr[idx]);
        CHECK_EVENT(I2C_EVENT_MASTER_BYTE_TRANSMITTED);
    }
    I2C_GenerateSTOP(I2C1, ENABLE);
    addr += count;
    dataPtr += count;
    dataLen -= count;

    /* 把剩下的数据写入 */
    uint8_t pages = (dataLen + 7) / 8;
    uint8_t p;
    for(p = 0; p < pages; ++p){
        delay_ms(5);
        CHECK_FLAG(I2C_FLAG_BUSY);
        I2C_GenerateSTART(I2C1, ENABLE);
        CHECK_EVENT(I2C_EVENT_MASTER_MODE_SELECT);
        I2C_Send7bitAddress(I2C1, EEPROM_ADDRESS, I2C_Direction_Transmitter);
        CHECK_EVENT(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
        I2C_SendData(I2C1, addr);
        CHECK_EVENT(I2C_EVENT_MASTER_BYTE_TRANSMITTING);
        for(int i = 0; i < 8; ++i){
            idx = i + p * 8;
            if(idx >= dataLen) 
                break;
            I2C_SendData(I2C1, dataPtr[idx]);
            CHECK_EVENT(I2C_EVENT_MASTER_BYTE_TRANSMITTED);
        }
        I2C_GenerateSTOP(I2C1, ENABLE);
    }
}

void EEPROM_Read_Sequential_IH(uint8_t addr, uint8_t* dataPtr, uint8_t dataLen){
    if((uint16_t)addr + (uint16_t)dataLen > 0XFF)
        fprintf(stderr, "Read bytes overflowed, line: %d, function: %s\r\n", __LINE__, __FUNCTION__);

    /* 等待上一次的I2C信号结束 */
    CHECK_FLAG(I2C_FLAG_BUSY);

    /* 读取多个数据 */
    I2C_GenerateSTART(I2C1, ENABLE);
    CHECK_EVENT(I2C_EVENT_MASTER_MODE_SELECT);
    I2C_Send7bitAddress(I2C1, EEPROM_ADDRESS, I2C_Direction_Transmitter);
    CHECK_EVENT(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
    I2C_SendData(I2C1, addr);
    CHECK_EVENT(I2C_EVENT_MASTER_BYTE_TRANSMITTED);
    I2C_GenerateSTART(I2C1, ENABLE);
    CHECK_EVENT(I2C_EVENT_MASTER_MODE_SELECT);
    I2C_Send7bitAddress(I2C1, EEPROM_ADDRESS, I2C_Direction_Receiver);
    CHECK_EVENT(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);
    uint8_t idx;
    for(idx = 0; idx < dataLen - 1; ++idx){
        CHECK_EVENT(I2C_EVENT_MASTER_BYTE_RECEIVED);    // 注意一定要先等待EV7再读取DR寄存器，否则读取错误数据
        dataPtr[idx] = I2C_ReceiveData(I2C1);
    }
    I2C_AcknowledgeConfig(I2C1, DISABLE);           // 最后一个receive信号之前关闭ACK信号
    CHECK_EVENT(I2C_EVENT_MASTER_BYTE_RECEIVED);    // 注意一定要先等待EV7再读取DR寄存器，否则读取错误数据
    dataPtr[idx] = I2C_ReceiveData(I2C1);
    I2C_GenerateSTOP(I2C1, ENABLE);

    /* 等待stop信号完成后重新使能ACK信号 */
    CHECK_FLAG(I2C_FLAG_BUSY);
    I2C_AcknowledgeConfig(I2C1, ENABLE);
}

extern void USART1_Init();

void I2C_Hardware_Run(){
    delay_init();
    USART1_Init();
    I2C_Hardware_Init();

    /* Check */
    Check_EEROM_IH();

    const char src[10] = "A1234567";
    char dst[10] = "";
    uint8_t addr = 100;

    /* Byte write */
    EEPROM_Write_Byte_IH(addr, (uint8_t)src[0]);
    printf("Write byte success\r\n");

    /* Byte read */
    dst[0] = EEPROM_Read_Byte_IH(addr);
    printf("Read byte success: %c\r\n", dst[0]);

    /* Sequential Write */
    EEPROM_Write_Sequential_IH(addr, (uint8_t*)src, 8);
    printf("Write sequential success\r\n");

    /* Sequential Read */
    EEPROM_Read_Sequential_IH(addr, (uint8_t*)dst, 8);
    printf("Read sequential success: %s\r\n", dst);
}