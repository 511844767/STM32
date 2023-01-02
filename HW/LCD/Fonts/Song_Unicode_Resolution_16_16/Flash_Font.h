#ifndef FLASH_FONT_H__
#define FLASH_FONT_H__
#include"stdlib.h"
#include"string.h"
#include"exFlash.h"
/*
串口输出：
    SD卡挂载成功
    Flash正在做擦除准备...
    Flash擦除准备完成
    正在写入文件...
    文件写入成功：0:Fonts/eng.bin
        内存范围[ 0 - 0x1036 ]
    文件写入成功：0:Fonts/math.bin
        内存范围[ 0x1037 - 0x306d ]
    文件写入成功：0:Fonts/zh.bin
        内存范围[ 0x306e - 0xa6564 ]
    文件写入成功：0:Fonts/zh_symbol.bin
        内存范围[ 0xa6565 - 0xa839b ]
    文件读写完成，正在校验文件...
    文件校验合格：0:Fonts/eng.bin
        内存范围[ 0 - 0x1036 ]
    文件校验合格：0:Fonts/math.bin
        内存范围[ 0x1037 - 0x306d ]
    文件校验合格：0:Fonts/zh.bin
        内存范围[ 0x306e - 0xa6564 ]
    文件校验合格：0:Fonts/zh_symbol.bin
        内存范围[ 0xa6565 - 0xa839b ]
    所有文件校验合格，本次烧录完成
*/

/* 字模尺寸 */
#define FLASH_FONT_HEIGHT   16  
#define FLASH_FONT_WIDTH    16
#define FLASH_FONT_REAL_SIZE (FLASH_FONT_HEIGHT * FLASH_FONT_WIDTH / 8)         // Flash内存
#define FLASH_FONT_GRID_SIZE ((FLASH_FONT_HEIGHT * FLASH_FONT_WIDTH + 7) / 8)   // CPU内存对齐到1Byte

/* 字模位置信息 */
#define ENG_FLASH_OFFSET        0       // 烧录位置
#define ENG_UNICODE_START       0x0000  // 编码起点
#define ENG_UNICODE_END         0x007F  // 编码终点
#define MATH_FLASH_OFFSET       0x1037
#define MATH_UNICODE_START      0x2200
#define MATH_UNICODE_END        0x22FF
#define ZH_FLASH_OFFSET         0x306e    
#define ZH_UNICODE_START        0x4E00
#define ZH_UNICODE_END          0x9FA5
#define ZH_SYMBOL_FLASH_OFFSET  0xa6565    
#define ZH_SYMBOL_UNICODE_START 0xFF00
#define ZH_SYMBOL_UNICODE_END   0xFFEF


/**
 * @brief utf-8解码为字模
 * 
 * @param[out] dstBuffer 字模
 * @param[in] srcBuffer 字符
 * @return 字符编码后的长度
 */
uint32_t FLASH_Font_DeCode(uint8_t* dstBuffer, const char* srcBuffer);

#endif