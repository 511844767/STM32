#include"stdbool.h"
#include"stdio.h"
#include"stm32f10x.h"
#include"delay.h"

/**
 * @brief 用GPIO仿真I2C时序，读写EEPROM
 *  PB6连SCL，PB7连SDA
 *  I2C协议的特性要求输出为开漏输出：低电平靠N-MOS导通接地，高电平靠上拉
 *  EEPROM AT24C02有2048Byte内存，需要8bit取址
 *  EEPROM AT24C02要求信号从高位开始发送（其实只要求把地址高位开始发送，而data可以强行从低位开始发送）
 */
void I2C_Software_Init(){
    /* 使能GPIOB时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    /* 配置GPIO */
    GPIO_InitTypeDef gpioDef;
    gpioDef.GPIO_Mode   = GPIO_Mode_Out_OD; // 开漏输出
    gpioDef.GPIO_Pin    = GPIO_Pin_6;
    gpioDef.GPIO_Speed  = GPIO_Speed_50MHz; // 50MHz满足I2C的400KHz的频率
    GPIO_Init(GPIOB, &gpioDef);
    gpioDef.GPIO_Pin    = GPIO_Pin_7;
    GPIO_Init(GPIOB, &gpioDef);
}

/* 控制SDA和SCL的宏，全部换成寄存器宏，提高读写性能 */
#define __GPIO_SetBits(GPIOx, GPIO_Pin)             GPIOx->BSRR = GPIO_Pin
#define __GPIO_ResetBits(GPIOx, GPIO_Pin)           GPIOx->BRR = GPIO_Pin
#define __GPIO_ReadInputDataBit(GPIOx, GPIO_Pin)    ((GPIOx->IDR & GPIO_Pin)? 1: 0)
#define SDASet1()  __GPIO_SetBits(GPIOB, GPIO_Pin_7)
#define SDASet0()  __GPIO_ResetBits(GPIOB, GPIO_Pin_7)
#define SCLSet1()  __GPIO_SetBits(GPIOB, GPIO_Pin_6)
#define SCLSet0()  __GPIO_ResetBits(GPIOB, GPIO_Pin_6)
#define SDAGet()   __GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7)
#define SCLGet()   __GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6)

/* 时序宽度，从EEPROM的Data Sheet中算出 */
#define Signal_Delay() delay_us(2)  // t_low, t_high高低电平最短有效时间
#define WR_Delay()     delay_ms(5)  // t_wr前后两次信号最短间隔

/* 发送起始信号 */
void I2C_SendSignal_Start(){
    SDASet1(); SCLSet1();
    Signal_Delay();
    SDASet0();
    Signal_Delay();
    SCLSet0();
    Signal_Delay();
}                               // 起始信号之后还是主机说话，不需要释放SDA控制权

/* 发送结束信号 */
void I2C_SendSignal_Stop(){
    SDASet0(); SCLSet1();
    Signal_Delay();
    SDASet1();
    Signal_Delay();
}                               // 结束信号之后的SDA控制权已释放

/* 发送应答信号 */
void I2C_SendSignal_ACK(){
    SDASet0(); SCLSet1();
    Signal_Delay();
    SCLSet0();
    SDASet1();                  // 应答之后主机要释放SDA控制权，让从机说话
    Signal_Delay();
}

/* 发送非应答信号 */
void I2C_SendSignal_NO_ACK(){
    SDASet1(); SCLSet1();
    Signal_Delay();
    SCLSet0();
    Signal_Delay();
}                               // 非应答之后的SDA控制权已释放


/* 发送一个字节的数据信号 */
void I2C_SendSignal_DataByte(uint8_t data){
    int i;
    for(i = 7; i >= 0; --i){
        if(data & (1 << i))     // 从高位开始发送
            SDASet1();
        else
            SDASet0();
        SCLSet1();
        Signal_Delay();
        SCLSet0();
        if(i == 0) SDASet1();   // 最后一个信号发完就要释放SDA控制权
        Signal_Delay();
    }
}

/* 接收应答/非应答信号 */
bool I2C_GetSignal_IS_ACK(){
    SDASet1(); SCLSet1();           // 接收信号之前要释放SDA控制权
    Signal_Delay();
    bool ACKflag = (SDAGet() == 0); // 低电平为ACK，高电平为NO ACK
    SCLSet0();
    Signal_Delay();
    return ACKflag;
}

/* 接收一个字节的数据信号 */
uint8_t I2C_GetSignal_DataByte(){
    uint8_t data = 0;
    int i;
    for(i = 7; i >= 0; --i){
        SDASet1(); SCLSet1();       // 接收信号之前释放SDA控制权
        Signal_Delay();
        data |= (SDAGet() << i);    // 从高位开始读取
        SCLSet0();
        Signal_Delay();
    }
    return data;
}

/* 设备地址，0位根据读/写确定，1~3位从EEPROM电路图的A0~A2引脚确定，4~7位从Data Sheet查得 */
#define READ_ADDRESS    0B10100001
#define WRITE_ADDRESS   0B10100000

/* 串口初始化 */
extern void USART1_Init();

/* 应答检查，TODO: 报错后怎么结束进程？ */
#define CHECK_ACK(info) \
if(!I2C_GetSignal_IS_ACK()){ \
    fprintf(stderr, "%s, line: %d, function: %s\r\n", info, __LINE__, __FUNCTION__); \
    I2C_SendSignal_Stop(); \
}
#define CHECK_NO_ACK(info) \
if(I2C_GetSignal_IS_ACK()){ \
    fprintf(stderr, "%s, line: %d, function: %s\r\n", info, __LINE__, __FUNCTION__); \
    I2C_SendSignal_Stop(); \
}

/* 检测EEPROM是否存在 */
void Check_EEPROM(){
    I2C_SendSignal_Start();                 // 开始
    I2C_SendSignal_DataByte(WRITE_ADDRESS); // 发送一个写地址（注意不能发送读地址，因为Read信号无法马上终止）
    CHECK_ACK("Not Found EEPROM");     // 检测EEPROM对读地址是否有响应
    I2C_SendSignal_Stop();                  // 结束
}

/* 写入一个字节 */
void EEPROM_Write_Byte(uint8_t addr, uint8_t data){
    I2C_SendSignal_Start();
    I2C_SendSignal_DataByte(WRITE_ADDRESS);
    CHECK_ACK("Write Failed");
    I2C_SendSignal_DataByte(addr);  // 写入地址
    CHECK_ACK("Write Failed");
    I2C_SendSignal_DataByte(data);   // 写入内容
    CHECK_ACK("Write Failed");
    I2C_SendSignal_Stop();

    WR_Delay();
}

/* 读出一个字节 */
uint8_t EEPROM_Read_Byte(uint8_t addr){
    I2C_SendSignal_Start();
    I2C_SendSignal_DataByte(WRITE_ADDRESS);
    CHECK_ACK("Read Failed");
    I2C_SendSignal_DataByte(addr);  // 读取地址
    CHECK_ACK("Read Failed");
    I2C_SendSignal_Start();
    I2C_SendSignal_DataByte(READ_ADDRESS);
    CHECK_ACK("Read Failed");
    uint8_t data = I2C_GetSignal_DataByte();
    CHECK_NO_ACK("Read Failed");
    I2C_SendSignal_Stop();

    WR_Delay();
    return data;
}

/* 写入多个数据 */
void EEPROM_Write_Sequential_IS(uint8_t addr, uint8_t* dataPtr, uint8_t dataLen){
    if((uint16_t)addr + (uint16_t)dataLen > 0XFF)
        fprintf(stderr, "Write bytes overflowed, line: %d, function: %s\r\n", __LINE__, __FUNCTION__);
    /* 写到addr对齐page */
#define MIN(a, b) (a < b)? a: b 
    uint8_t count = MIN(8 - addr % 8, dataLen);
    uint8_t idx;
    I2C_SendSignal_Start();
    I2C_SendSignal_DataByte(WRITE_ADDRESS);
    CHECK_ACK("Write Failed");
    I2C_SendSignal_DataByte(addr);  // 写入地址
    CHECK_ACK("Write Failed");
    for(idx = 0; idx < count; ++idx){
        I2C_SendSignal_DataByte(dataPtr[idx]);  // 写入数据
        CHECK_ACK("Write Failed");
    }
    I2C_SendSignal_Stop();
    WR_Delay();
    addr += count;
    dataPtr += count;
    dataLen -= count;

    /* 把剩下的数据写入 */
    uint8_t pages = (dataLen + 7) / 8;
    uint8_t p;
    for(p = 0; p < pages; ++p){
        I2C_SendSignal_Start();
        I2C_SendSignal_DataByte(WRITE_ADDRESS);
        CHECK_ACK("Write Failed");
        I2C_SendSignal_DataByte(addr + p * 8);  // 写入地址
        CHECK_ACK("Write Failed");
        for(int i = 0; i < 8; ++i){
            idx = i + p * 8;
            if(idx >= dataLen) 
                break;
            I2C_SendSignal_DataByte(dataPtr[idx]);  // 写入数据
            CHECK_ACK("Write Failed");
        }
        I2C_SendSignal_Stop();
        WR_Delay();
    }
}

/* 读取多个数据 */
void EEPROM_Read_Sequential_IS(uint8_t addr, uint8_t* dataPtr, uint8_t dataLen){
    if((uint16_t)addr + (uint16_t)dataLen > 0XFF)
        fprintf(stderr, "Read bytes overflowed, line: %d, function: %s\r\n", __LINE__, __FUNCTION__);
    I2C_SendSignal_Start();
    I2C_SendSignal_DataByte(WRITE_ADDRESS);
    CHECK_ACK("Read Failed");
    I2C_SendSignal_DataByte(addr);  // 读取地址
    CHECK_ACK("Read Failed");
    I2C_SendSignal_Start();
    I2C_SendSignal_DataByte(READ_ADDRESS);
    CHECK_ACK("Read Failed");
    uint8_t idx;
    for(idx = 0; idx < dataLen - 1; ++idx){
        dataPtr[idx] = I2C_GetSignal_DataByte();
        I2C_SendSignal_ACK();
    }
    dataPtr[idx] = I2C_GetSignal_DataByte();
    CHECK_NO_ACK("Read Failed");
    I2C_SendSignal_Stop();

    WR_Delay();
}

/* 测试例程 */
void I2C_Software_Run(){
    /* 初始化 */
    delay_init();
    USART1_Init();
    I2C_Software_Init();

    /* 检测EEPROM */
    Check_EEPROM();
    printf("Founded EEPROM\r\n");

    char srcBuffer[10] = "A1234567";
    char dstBuffer[10] = "";

    /* Byte Write */
    EEPROM_Write_Byte(0X00, (uint8_t)srcBuffer[0]);
    printf("Write byte success\r\n");
    
    /* Random Read */
    dstBuffer[0] = (char)EEPROM_Read_Byte(0X00);
    printf("Read byte success: %c\r\n", dstBuffer[0]);

    /* Sequential Write */
    EEPROM_Write_Sequential_IS(0X50, (uint8_t*)srcBuffer, 8);
    printf("Write sequential success\r\n");

    /* Sequential Read */
    EEPROM_Read_Sequential_IS(0X50, (uint8_t*)dstBuffer, 8);
    printf("Read sequential success: %s\r\n", dstBuffer);
}