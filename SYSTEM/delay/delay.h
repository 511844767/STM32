#ifndef __DELAY_H
#define __DELAY_H 			   
#include "sys.h"  
	 
void delay_init(void);
void delay_ms(u16 nms);
void delay_us(u32 nus);
void delay_s(uint32_t s);

/**
 * @brief 纳秒级延时（72MHz）
 * @note __NOP消耗1个时钟周期（72MHz下为13.888ns），这些延迟的误差为-1ns~0ns
 * 
 */
#define delay_14ns(){ \
    __NOP(); \
}   // 1 * NOP
#define delay_55ns(){ \
    __NOP(); \
    __NOP(); \
    __NOP(); \
    __NOP(); \
}   // 4 * NOP
#define delay_111ns(){ \
    __NOP(); \
    __NOP(); \
    __NOP(); \
    __NOP(); \
    __NOP(); \
    __NOP(); \
    __NOP(); \
    __NOP(); \
}   // 8 * NOP
#define delay_208ns(){ \
    __NOP(); \
    __NOP(); \
    __NOP(); \
    __NOP(); \
    __NOP(); \
    __NOP(); \
    __NOP(); \
    __NOP(); \
    __NOP(); \
    __NOP(); \
    __NOP(); \
    __NOP(); \
    __NOP(); \
    __NOP(); \
    __NOP(); \
}   // 15 * NOP

#endif





























