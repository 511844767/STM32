#include"ff.h"
#include"stdio.h"
#include"stdlib.h"

/* 异常检查 */
typedef struct{
    const char info[72];
} FATFS_Errors_Info_t;

static FATFS_Errors_Info_t errInfo[20] = {
    { "(0) Succeeded" },
    { "(1) A hard error occurred in the low level disk I/O layer" },
    { "(2) Assertion failed" },
    { "(3) The physical drive cannot work" },
    { "(4) Could not find the file" },
    { "(5) Could not find the path" },
    { "(6) The path name format is invalid" },
    { "(7) Access denied due to prohibited access or directory full" },
    { "(8) Access denied due to prohibited access" },
    { "(9) The file/directory object is invalid" },
    { "(10) The physical drive is write protected" },
    { "(11) The logical drive number is invalid" },
    { "(12) The volume has no work area" },
    { "(13) There is no valid FAT volume" },
    { "(14) The f_mkfs() aborted due to any problem" },
    { "(15) Could not get a grant to access the volume within defined perio" },
    { "(16) The operation is rejected according to the file sharing policy" },
    { "(17) LFN working src could not be allocated" },
    { "(18) Number of open files > FF_FS_LOCK" },
    { "(19) Given parameter is invalid" },
};

#define FATFS_CHECK(func){ \
    err = func; \
    if(err != FR_OK){ \
        fprintf(stderr, "Error line: %d, function: %s, info: %s\r\n", __LINE__, __FUNCTION__, errInfo[err].info); \
        goto FATFS_ERR_FLAG; \
    } \
}

void Flash_EX_FatFs_Run(){
    FRESULT err = 0;
    FIL* fp = 0;

    /* 挂载flash */
    /* FATFS和FIL结构体内存较大，放在堆空间比较合适 */
    FATFS* fs = malloc(sizeof(FATFS));
    err = f_mount(fs, "1:", 1);                 // path为diskio.c中定义的DEV_FLASH_EX，opt为1表示立即挂载

    /* 第一次挂载如果没找到FatFs文件系统，则格式化加载文件系统 */
    if(err == FR_NO_FILESYSTEM){ 
        printf("未加载FatFs系统，需要格式化\r\n");
        uint8_t* work = malloc(4096);           // 格式化工作空间
        FATFS_CHECK(f_mkfs("1:", NULL, work, 4096));
        free(work);
        FATFS_CHECK(f_mount(fs, "1:", 1));      // 格式化之后重新挂载
    }else if(err != FR_OK){
        fprintf(stderr, "Error line: %d, function: %s, info: %s\r\n", __LINE__, __FUNCTION__, errInfo[err].info);
        goto FATFS_ERR_FLAG;
    }

    /* 打开文件 */
    fp = malloc(sizeof(FIL));
    FATFS_CHECK(f_open(fp, "1:测试.txt", FA_CREATE_ALWAYS | FA_READ | FA_WRITE));

    /* 写入文件 */
    const char src[20] = "Diana Ava";
    UINT bw;
    FATFS_CHECK(f_write(fp, src, 20, &bw));
    if(bw != 20){   // bw返回实际写入长度，应该等于理论写入长度
        fprintf(stderr, "Error line: %d, function: %s, info: write error\r\n", __LINE__, __FUNCTION__);
        goto FATFS_ERR_FLAG;
    }

    /* 关闭文件 */
    FATFS_CHECK(f_close(fp));

    printf("文件写入成功\r\n");

    /* 读取文件 */
    FATFS_CHECK(f_open(fp, "1:测试.txt", FA_READ | FA_OPEN_EXISTING));
    char dst[20];
    UINT br;
    FATFS_CHECK(f_read(fp, dst, 20, &br));
    if(br != 20){   // br返回实际读取长度，应该等于理论读取长度
        fprintf(stderr, "Error line: %d, function: %s, info: read error\r\n", __LINE__, __FUNCTION__);
        goto FATFS_ERR_FLAG;
    }

    /* 关闭文件 */
    FATFS_CHECK(f_close(fp));

    printf("文件读取成功：%s\r\n", dst);
FATFS_ERR_FLAG:
    if(fp) free(fp);
    if(fs) free(fs);
}