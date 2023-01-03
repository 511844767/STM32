#include"math.h"
#include"string.h"
#include"lcd_utils.h"
#include"Flash_Font.h"

void LCD_Init(){
    /* 使能时钟源 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG | RCC_APB2Periph_GPIOD | 
        RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOB, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
    /* 配置FSMC对应的GPIO引脚（参考手册表30） */
    LCD_CONFIG_GPIO();
    /* 配置FSMC */
    LCD_CONFIG_FSMC();
    /* 使能FSMC */
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);
    delay_ms(50);   // 初始化之后等待一会，否则读写不稳定
    /* 发送LCD初始化序列 */
    LCD_INIT_SEQUENCE();
    /* 点亮背光 */
    LCD_BL_ON();
}

/* 读取ID */
void LCD_Read_ID(){
    LCD_Write_Cmd(0XD4);
    LCD_Read_Dummy();
    uint16_t Vender_ID      = LCD_Read_Data();
    uint16_t Chip_ID        = LCD_Read_Data();
    Chip_ID                 = (Chip_ID << 8) | LCD_Read_Data();
    uint16_t Chip_Version   = LCD_Read_Data();
    printf("Vender ID       = %x\r\n", Vender_ID);
    printf("Chip ID         = %x\r\n", Chip_ID);
    printf("Chip Version    = %x\r\n", Chip_Version);
}

/* 读取像素格式 */
void LCD_Read_Pixel_Format(){
    LCD_Write_Cmd(0X0C);
    LCD_Read_Dummy();
    uint16_t data = LCD_Read_Data();
    printf("Pixel Format    = %x\r\n", data);
}

/* 设置窗口范围 */
void __LCD_Set_Window(LCD_Rectangle_t rect){
    /* 设置x范围 */
    LCD_Write_Cmd(0X2A);
    LCD_Write_Data((rect.x1 & 0XFF00) >> 8);
    LCD_Write_Data((rect.x1 & 0X00FF) >> 0);
    LCD_Write_Data((rect.x2 & 0XFF00) >> 8);
    LCD_Write_Data((rect.x2 & 0X00FF) >> 0);
    /* 设置y范围 */
    LCD_Write_Cmd(0X2B);
    LCD_Write_Data((rect.y1 & 0XFF00) >> 8);
    LCD_Write_Data((rect.y1 & 0X00FF) >> 0);
    LCD_Write_Data((rect.y2 & 0XFF00) >> 8);
    LCD_Write_Data((rect.y2 & 0X00FF) >> 0);
}

/* 绘制实心矩形 */
void LCD_Draw_SolidRect(LCD_Rectangle_t rect, LCD_Color_t color){
    /* 裁剪矩形范围 */
    rect.x1 = LCD_MIN(rect.x1, LCD_WIDTH - 1);
    rect.x2 = LCD_MIN(rect.x2, LCD_WIDTH - 1);
    rect.y1 = LCD_MIN(rect.y1, LCD_HEIGHT - 1);
    rect.y2 = LCD_MIN(rect.y2, LCD_HEIGHT - 1);
    uint32_t width  = rect.x2 - rect.x1 + 1;
    uint32_t height = rect.y2 - rect.y1 + 1;
    /* 设置窗口 */
    __LCD_Set_Window(rect);
    /* 写入像素点 */
    LCD_Write_Cmd(0X2C);
    for(uint32_t i = 0; i < width * height; ++i){
        LCD_Write_Data(color);
    }
}

/* 填充屏幕 */
void LCD_Fill(LCD_Color_t color){
    LCD_Rectangle_t rect = {0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1};
    LCD_Draw_SolidRect(rect, color);
}

/* 画点 */
void LCD_Draw_Point(uint16_t x, uint16_t y, LCD_Color_t color){
    if(x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    /* 设置x点 */
    LCD_Write_Cmd(0X2A);
    LCD_Write_Data((x & 0XFF00) >> 8);
    LCD_Write_Data((x & 0X00FF) >> 0);
    LCD_Write_Data((x & 0XFF00) >> 8);
    LCD_Write_Data((x & 0X00FF) >> 0);
    /* 设置y点 */
    LCD_Write_Cmd(0X2B);
    LCD_Write_Data((y & 0XFF00) >> 8);
    LCD_Write_Data((y & 0X00FF) >> 0);
    LCD_Write_Data((y & 0XFF00) >> 8);
    LCD_Write_Data((y & 0X00FF) >> 0);
    /* 填色 */
    LCD_Write_Cmd(0X2C);
    LCD_Write_Data(color);
}

/**
 * @brief Bresenham算法绘制直线
 * @note  无法支持线宽，一般直接移植图形库即可，不需要自己写这些底层绘图算法
 *        常见的图形库例如：https://github.com/lvgl/lvgl
 * @remark 本来移植了OpenCV的直线绘制，结果不小心误删了，唉
 */
void LCD_Draw_Line(LCD_Line_t line, LCD_Color_t color){
    /* 如果直线长度为0，只有1个像素点 */
    if((line.x1 == line.x2) && (line.y1 == line.y2)){
        LCD_Draw_Point(line.x1, line.y1, color);
        return;
    }

    /* 如果直线水平 */
    LCD_Rectangle_t rect;
    if(line.y1 == line.y2){
        rect.x1 = LCD_MIN(line.x1, line.x2);
        rect.x2 = LCD_MAX(line.x1, line.x2);
        rect.y1 = rect.y2 = line.y1;
        LCD_Draw_SolidRect(rect, color);
        return;
    }

    /* 如果直线竖直 */
    if(line.x1 == line.x2){
        rect.x1 = rect.x2 = line.x1;
        rect.y1 = LCD_MIN(line.y1, line.y2);
        rect.y2 = LCD_MAX(line.y1, line.y2);
        LCD_Draw_SolidRect(rect, color);
        return;
    }

    LCD_Draw_Point(line.x1, line.y1, color); /* 绘制起点 */
    LCD_Draw_Point(line.x2, line.y2, color); /* 绘制终点 */

    uint16_t tmp;
#define SWAP(a, b) {tmp = a; a = b; b = tmp;}

    /* 将二、三象限的直线变换到一、四象限 */
    if(line.x1 > line.x2){
        SWAP(line.x1, line.x2);
        SWAP(line.y1, line.y2);
    }
    /* 如果在第四象限，沿x轴镜像翻转 */
    bool        xAxisMirrorFlipFlag     = (line.y2 < line.y1);
    uint16_t    xAxisCoordinateYAY      = (line.y1 + line.y2);  /* x轴的y坐标的两倍（用于计算镜像还原的值） */
    if(xAxisMirrorFlipFlag){ SWAP(line.y1, line.y2); }
    /* 如果斜率大于45°，则沿y=x轴镜像翻转 */
    bool        yExLineMirrorFlipFlag   = ((line.y2 - line.y1) > (line.x2 - line.x1));
    uint16_t    yExLineMirrorDeltaX, yExLineMirrorDeltaY;       /* 相对坐标的原点位置 */
    if(yExLineMirrorFlipFlag){
        yExLineMirrorDeltaX = line.x1; 
        yExLineMirrorDeltaY = line.y1;
        line.x1             -= yExLineMirrorDeltaX;             /* 绝对坐标转换为相对坐标 */
        line.y1             -= yExLineMirrorDeltaY;
        line.x2             -= yExLineMirrorDeltaX; 
        line.y2             -= yExLineMirrorDeltaY;
        SWAP(line.x1, line.y1);
        SWAP(line.x2, line.y2);
    }

    /* 绘制第一象限、斜率小于45°的直线 */
    uint16_t    dx = line.x2 - line.x1, dy = line.y2 - line.y1;                         /* 斜率信息 */
    int16_t     incrE = 2 * (int16_t)dy, incrNE = 2 * ((int16_t)dy - (int16_t)dx);      /* 条件增长 */
    int16_t     d = 2 * dy - dx;                                                        /* 判断条件 */
    uint16_t    xRelative = line.x1 + 1, yRelative = line.y1;                           /* 相对坐标 */
    uint16_t    xAbsolute, yAbsolute;                                                   /* 绝对坐标 */
    for(; xRelative < line.x2; ++xRelative){                                            /* 遍历x方向 */
        if(d < 0){
            d += incrE;     /* go East */
        }    
        else{
            d += incrNE;    /* go NorthEast */
            ++yRelative;
        }
        if(yExLineMirrorFlipFlag){                                                      /* 还原斜率大于45° */
            xAbsolute = yRelative + yExLineMirrorDeltaX;
            yAbsolute = xRelative + yExLineMirrorDeltaY;
        } else{
            xAbsolute = xRelative;
            yAbsolute = yRelative;
        }
        if(xAxisMirrorFlipFlag){ yAbsolute = xAxisCoordinateYAY - yAbsolute; }          /* 还原第四象限 */
        LCD_Draw_Point(xAbsolute, yAbsolute, color);
    }
#undef SWAP
}


/**
 * @brief 绘制文字（utf-8编码）,支持常用中文、英文、数学符号、标点符号、换行
 * @note 除了换行之外的转译字符无法正常绘制
 * TODO: 字模可以通过上、下采样来缩放字体大小
 * @param x 文字框左上角x坐标
 * @param y 文字框左上角y坐标
 * @param text 文字内容
 */
void LCD_Draw_Text(uint16_t x, uint16_t y, const char* text, LCD_Color_t fg, LCD_Color_t bg){
    uint8_t buffer[FLASH_FONT_GRID_SIZE];
    uint32_t finishedLen = 0, acc;
    LCD_Rectangle_t rect;
    /* 文字框滑动距离，区分英文、中文字符的大小不一问题 */
    extern uint16_t font_slop_x, font_slop_y;
    rect.x1 = x; rect.y1 = y; rect.x2 = x + FLASH_FONT_WIDTH - 1; rect.y2 = y + FLASH_FONT_HEIGHT - 1;
    size_t len = strlen(text);
    
    while(finishedLen <= len){
        /* 字符解码 */
        acc = FLASH_Font_DeCode(buffer, text);
        /* 写入像素点 */
        if(rect.x1 <= LCD_WIDTH && rect.y1 <= LCD_HEIGHT){  // 在边界内
            /* 设置窗口 */
            __LCD_Set_Window(rect);
            LCD_Write_Cmd(0X2C);
            for(uint32_t i = 0; i < FLASH_FONT_REAL_SIZE * 8; ++i){
                if(buffer[i / 8] & (0B10000000 >> (i % 8))){
                    LCD_Write_Data(fg);
                }else{
                    LCD_Write_Data(bg);
                }
            }
        }
        /* 移动绘图点 */
        if(text[0] == '\n'){    // 换行
            rect.x1 = x; rect.y1 += FLASH_FONT_HEIGHT;
            rect.x2 = x + FLASH_FONT_WIDTH - 1; rect.y2 += FLASH_FONT_HEIGHT;
        }else{                  // 前进一格
            rect.x1 += font_slop_x; rect.x2 += font_slop_x;
            rect.y1 += font_slop_y; rect.y2 += font_slop_y;
        }
        text += acc;
        finishedLen += acc;
    }
}

extern void USART1_Init();

void LCD_Run(){
    delay_init();
    LCD_Init();
    USART1_Init();

    /* 打印LCD ID */
    LCD_Read_ID();
    /* 打印像素格式 */
    LCD_Read_Pixel_Format();
    /* 清空屏幕 */
    LCD_Fill(0XFFFF);
    /* 绘制实心矩形 */
    LCD_Rectangle_t rect = {50, 50, 100, 100};
    LCD_Draw_SolidRect(rect, LCD_Color_RGB565(0, 0, 0));
    /* 绘制直线 */
    LCD_Line_t xAxis = {0, 200, LCD_WIDTH, 200};
    LCD_Draw_Line(xAxis, LCD_Color_RGB565(0, 0, 0));
    LCD_Line_t yAxis = {200, 0, 200, LCD_HEIGHT};
    LCD_Draw_Line(yAxis, LCD_Color_RGB565(0, 0, 0));
    LCD_Line_t line;
    line.x1 = 200; line.y1 = 200;
#define LCD_PI		3.14159265358979323846
    for(uint16_t i = 0; i < 360; i += 10){
        line.x2 = (uint16_t)((double)line.x1 + 100 * cos((double)i / 180 * LCD_PI));
        line.y2 = (uint16_t)((double)line.y1 + 100 * sin((double)i / 180 * LCD_PI));
        LCD_Draw_Line(line, LCD_Color_RGB565(0X1F, 0, 0));
    }
    /* 绘制文字 */
    Flash_EX_Init();
    LCD_Draw_Text(200, 50, "Diana + Ava\n文字测试", LCD_Color_RGB565(0, 0, 0), LCD_Color_RGB565(255, 255, 255));
    while(1);
}