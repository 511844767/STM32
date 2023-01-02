#include"Flash_Font.h"

/* 解码错误的字符 */
static uint8_t dummyCharacter[FLASH_FONT_GRID_SIZE] = {0XFF};

/* 文字框滑动距离，区分英文、中文每个字符的大小问题 */
uint16_t font_slop_x = FLASH_FONT_WIDTH;
uint16_t font_slop_y = 0;

/**
 * @brief utf-8解码为unicode
 * @return 返回utf-8长度，如果返回0，表示解码错误
 */
static uint8_t Flash_Utf8_To_Unicode(uint32_t* unicode, uint8_t* utf_8){
    /* 判断字符长度 */
    uint8_t i;
    for(i = 0; i < 6; ++i){
        if((utf_8[0] & (0B10000000 >> i)) == 0) break;
    }

    /* 根据字符长度选择解码方式 */
    switch (i)
    {
    case 0: // 0xxxxxxx
        *unicode = \
            ((utf_8[0] & 0B01111111) <<  0);
        return 1;

    case 2: // 110xxxxx 10xxxxxx
        *unicode = \
            ((utf_8[0] & 0B00011111) << 6) | \
            ((utf_8[1] & 0B00111111) <<  0);
        return 2;

    case 3: // 1110xxxx 10xxxxxx 10xxxxxx
        *unicode = \
            ((utf_8[0] & 0B00001111) << 12) | \
            ((utf_8[1] & 0B00111111) <<  6) | \
            ((utf_8[2] & 0B00111111) <<  0);
        return 3;

    case 4: // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        *unicode = \
            ((utf_8[0] & 0B00000111) << 18) | \
            ((utf_8[1] & 0B00111111) << 12) | \
            ((utf_8[2] & 0B00111111) <<  6) | \
            ((utf_8[3] & 0B00111111) <<  0);
        return 4;

    case 5: // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        *unicode = \
            ((utf_8[0] & 0B00000011) << 24) | \
            ((utf_8[1] & 0B00111111) << 18) | \
            ((utf_8[2] & 0B00111111) << 12) | \
            ((utf_8[3] & 0B00111111) <<  6) | \
            ((utf_8[4] & 0B00111111) <<  0);
        return 5;

    case 6: // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        *unicode = \
            ((utf_8[0] & 0B00000001) << 30) | \
            ((utf_8[1] & 0B00111111) << 24) | \
            ((utf_8[2] & 0B00111111) << 18) | \
            ((utf_8[3] & 0B00111111) << 12) | \
            ((utf_8[4] & 0B00111111) <<  6) | \
            ((utf_8[5] & 0B00111111) <<  0);
        return 6;

    default: // 编码错误：遇到10xxxxxx或者不再utf-8范围内的编码
        *unicode = 0;
        return 0;
    }
}


uint32_t FLASH_Font_DeCode(uint8_t* dstBuffer, const char* srcBuffer){
    uint32_t unicode;
    uint32_t acc;
    /* 解码utf-8为unicode */
    acc = Flash_Utf8_To_Unicode(&unicode, (uint8_t*)srcBuffer);
    if(acc == 0){    // 解码错误，字符设置为dummyCharacter
        memcpy(dstBuffer, dummyCharacter, FLASH_FONT_GRID_SIZE);
        return 1;
    }else{           // 解码正常，从字库中取字模
        if((ENG_UNICODE_START <= unicode)  && (unicode <= ENG_UNICODE_END)){    // eng编码范围
            Flash_EX_Read(
                dstBuffer, 
                ENG_FLASH_OFFSET + (unicode - ENG_UNICODE_START) * FLASH_FONT_REAL_SIZE, 
                FLASH_FONT_REAL_SIZE
            );
            font_slop_x = FLASH_FONT_WIDTH / 2;
        }else if((MATH_UNICODE_START <= unicode) && (unicode <= MATH_UNICODE_END)){ // math编码范围
            Flash_EX_Read(
                dstBuffer, 
                MATH_FLASH_OFFSET + (unicode - MATH_UNICODE_START) * FLASH_FONT_REAL_SIZE, 
                FLASH_FONT_REAL_SIZE
            );
            font_slop_x = FLASH_FONT_WIDTH / 2;
        }else if((ZH_UNICODE_START <= unicode) && (unicode <= ZH_UNICODE_END)){ // zh编码范围
            Flash_EX_Read(
                dstBuffer, 
                ZH_FLASH_OFFSET + (unicode - ZH_UNICODE_START) * FLASH_FONT_REAL_SIZE, 
                FLASH_FONT_REAL_SIZE
            );
            font_slop_x = FLASH_FONT_WIDTH;
        }else if((ZH_SYMBOL_UNICODE_START <= unicode) && (unicode <= ZH_SYMBOL_UNICODE_END)){   // zh_symbol编码范围
            Flash_EX_Read(
                dstBuffer, 
                ZH_SYMBOL_FLASH_OFFSET + (unicode - ZH_SYMBOL_UNICODE_START) * FLASH_FONT_REAL_SIZE, 
                FLASH_FONT_REAL_SIZE
            );
            font_slop_x = FLASH_FONT_WIDTH;
        }else{  // 超出预设的编码范围，字符设置为dummyCharacter
            memcpy(dstBuffer, dummyCharacter, FLASH_FONT_GRID_SIZE);
            return 1;
        }
    }
    return acc;
}