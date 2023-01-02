/**
 * @file Flash_Download.c
 * @author 511844767
 * @brief 将字模从SD卡中读取出来（要求SD卡安装了FatFs文件系统），烧录到外部Flash中
 * @version 0.1
 * @date 2023-01-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */
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
#include"stdio.h"
#include"stdlib.h"
#include"ff.h"
#include"Flash_Font.h"
#include"exFlash.h"
#include"delay.h"
#include"KEY.h"


#define FATFS_CHECK(func){ \
    err = func; \
    if(err != FR_OK){ \
        fprintf(stderr, "Error line: %d, function: %s, err[%d]\r\n", __LINE__, __FUNCTION__, err); \
        goto FONTS_ERR_FLAG; \
    } \
}

#define MIN(a, b) ((a) <= (b)) ? (a) : (b)

typedef struct{
    const char path[50];
} PATH_t;

#define RW_GRID_MAX_SIZE 4096   /* 读写粒度（防止文件太大导致内存溢出） */

/* 文件信息 */
#define FILES_NUM 4
static PATH_t filesPath[FILES_NUM] = {
    {"0:Fonts/eng.bin"},
    {"0:Fonts/math.bin"},
    {"0:Fonts/zh.bin"},
    {"0:Fonts/zh_symbol.bin"}
};

#define FLASH_START_OFFSET 0    /* 从flash的什么位置开始写 */

extern void USART1_Init();

static FIL* fp;

static void Font_Burn(){
    FRESULT err = 0;
    uint8_t* dataSD = 0;
    Flash_EX_Errors_t flashErr;
    /* 擦除Flash */
    printf("\tFlash正在做擦除准备...\r\n");
    int i, j, size, gridNum, gridSize;
    size = 0;
    for(i = 0; i < FILES_NUM; ++i){
        FATFS_CHECK(f_open(fp, filesPath[i].path, FA_READ)); /* 打开文件 */
        size += f_size(fp); /* 文件总大小 */
        FATFS_CHECK(f_close(fp));
    }
    flashErr = Flash_EX_Erase_NonEmpty(FLASH_START_OFFSET, (uint32_t)size);
    if(flashErr != FLASH_EX_ERR_SUCCESS){
        printf("\tFlash擦除失败[line: %d]\r\n", __LINE__);
        goto FONTS_ERR_FLAG;
    }
    printf("\tFlash擦除准备完成\r\n");

    /* 读写 */
    printf("\t正在写入文件...\r\n");
    dataSD = (uint8_t*)malloc(RW_GRID_MAX_SIZE);
    if(!dataSD) {
        printf("\t内存分配失败：%dByte\r\n", RW_GRID_MAX_SIZE);
        goto FONTS_ERR_FLAG;
    }

    uint32_t addr = FLASH_START_OFFSET;  /* 文件起点 */
    UINT br;
    for(i = 0; i < FILES_NUM; ++i){
        FATFS_CHECK(f_open(fp, filesPath[i].path, FA_READ)); /* 打开文件 */
        f_rewind(fp);   /* 指向文件开头 */
        size = f_size(fp); /* 文件总大小 */
        gridNum = (size + RW_GRID_MAX_SIZE - 1) / RW_GRID_MAX_SIZE; /* 要分几次读写 */
        for(j = 0; j < gridNum; ++j){
            gridSize = MIN(RW_GRID_MAX_SIZE, size - j * RW_GRID_MAX_SIZE);  /* 本次读写大小 */
            FATFS_CHECK(f_lseek(fp, j * RW_GRID_MAX_SIZE));     /* 指针偏移 */
            FATFS_CHECK(f_read(fp, dataSD, gridSize, &br));        /* 读取文件 */
            if(br != gridSize){
                printf("\t文件读取失败[line: %d]\r\n", __LINE__);
                goto FONTS_ERR_FLAG;
            }
            flashErr = Flash_EX_Write(dataSD, addr + (uint32_t)(j * RW_GRID_MAX_SIZE), gridSize); /* 写入文件 */
            if(flashErr != FLASH_EX_ERR_SUCCESS){
                printf("\t文件写入失败[line: %d]\r\n", __LINE__);
                goto FONTS_ERR_FLAG;
            }
        }
        
        printf("\t文件写入成功：%s\r\n", filesPath[i].path);
        printf("\t\t内存范围[ %#x - %#x ]\r\n", (unsigned int)addr, (unsigned int)(addr + size - 1));
        addr += size;   /* 移动文件起点 */
        FATFS_CHECK(f_close(fp));
    }
FONTS_ERR_FLAG:
    if(dataSD)      free(dataSD);
}

static void Font_Check(){
    FRESULT err = 0;
    uint8_t* dataFlash = 0;
    uint8_t* dataSD = 0;
    int i, j, k, size, gridNum, gridSize;
    UINT br;
    Flash_EX_Errors_t flashErr;
    /* 校验 */
    printf("\t正在校验文件...\r\n");
    dataFlash = (uint8_t*)malloc(RW_GRID_MAX_SIZE);
    if(!dataFlash) {
        printf("\t内存分配失败：%dByte\r\n", RW_GRID_MAX_SIZE);
        goto FONTS_ERR_FLAG;
    }
    dataSD = (uint8_t*)malloc(RW_GRID_MAX_SIZE);
    if(!dataSD) {
        printf("\t内存分配失败：%dByte\r\n", RW_GRID_MAX_SIZE);
        goto FONTS_ERR_FLAG;
    }
    
    uint32_t addr = FLASH_START_OFFSET;  /* 读取开始点 */
    for(i = 0; i < FILES_NUM; ++i){
        FATFS_CHECK(f_open(fp, filesPath[i].path, FA_READ)); /* 打开文件 */
        f_rewind(fp);   /* 指向文件开头 */
        size = f_size(fp); /* 文件大小 */
        gridNum = (size + RW_GRID_MAX_SIZE - 1) / RW_GRID_MAX_SIZE; /* 要分几次读写 */
        for(j = 0; j < gridNum; ++j){
            gridSize = MIN(RW_GRID_MAX_SIZE, size - j * RW_GRID_MAX_SIZE);  /* 本次读写大小 */
            FATFS_CHECK(f_lseek(fp, j * RW_GRID_MAX_SIZE));     /* 指针偏移 */
            FATFS_CHECK(f_read(fp, dataSD, gridSize, &br));        /* 读取SD文件 */
            if(br != gridSize){
                printf("\t文件读取失败[line: %d]\r\n", __LINE__);
                goto FONTS_ERR_FLAG;
            }
            flashErr = Flash_EX_Read(dataFlash, addr + (uint32_t)(j * RW_GRID_MAX_SIZE), gridSize); /* 读取Flash文件 */
            
            if(flashErr != FLASH_EX_ERR_SUCCESS){
                printf("\t文件读取失败[line: %d]\r\n", __LINE__);
                goto FONTS_ERR_FLAG;
            }

            for(k = 0; k < gridSize; ++k){  /* 校验SD和Flash的内容 */
                if(dataSD[k] != dataFlash[k]){
                    printf("\t校验不合格\r\n");
                    goto FONTS_ERR_FLAG;
                }
            }
        }
        
        printf("\t文件校验合格：%s\r\n", filesPath[i].path);
        printf("\t\t内存范围[ %#x - %#x ]\r\n", (unsigned int)addr, (unsigned int)(addr + size - 1));
        addr += size;   /* 移动文件起点 */
        FATFS_CHECK(f_close(fp));
    }

    printf("\t所有文件校验合格\r\n");
FONTS_ERR_FLAG:
    if(dataSD)      free(dataSD);
    if(dataFlash)   free(dataFlash);
}

void Font_Download(){
    Flash_EX_Errors_t flashErr;

    /* 初始化 */
    delay_init();
    USART1_Init();
    KEY_Init();
    Flash_EX_Init();
    flashErr = Flash_EX_Wake_Up();
    if(flashErr != FLASH_EX_ERR_SUCCESS){
        printf("Flash唤醒失败\r\n");
        return;
    }
    bool flag;
    flashErr = Flash_EX_Check_JEDEC_ID(&flag);
    if((!flag) || flashErr != FLASH_EX_ERR_SUCCESS){
        printf("Flash ID错误\r\n");
        return;
    }

    /* 挂载SD卡 */
    FRESULT err = 0;
    FATFS* fs = malloc(sizeof(FATFS));
    FATFS_CHECK(f_mount(fs, "0:", 1)); // path为diskio.c中定义的DEV_SDCARD，opt为1表示立即挂载
    fp = malloc(sizeof(FIL));
    printf("SD卡挂载成功\r\n");

    /* 根据按键选择执行 */
#define KEY_PRINT() printf("按下Key0（烧录），Key1（校验），Key_up（烧录 + 校验）：\r\n")
    KEY_PRINT();
    while(1){
        KEY_STATUS_t key = KEY_Scan(KEY_SCAN_ONCE);
        if(key != KEY_NONE_PRESS){
            switch (key)
            {
            case KEY0_PRESS:    // 烧录
                Font_Burn();
                KEY_PRINT();
                break;
            case KEY1_PRESS:    // 校验
                Font_Check();
                KEY_PRINT();
                break;
            case KEYUP_PRESS:   // 烧录 + 校验
                Font_Burn();
                Font_Check();
                KEY_PRINT();
                break;
            default:
                break;
            }
        }
    }
#undef KEY_PRINT


FONTS_ERR_FLAG:
    if(fp)          free(fp);
    if(fs)          free(fs);
    return;
}