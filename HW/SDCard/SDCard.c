#include"stdio.h"
#include"stdlib.h"
#include"stm32_eval_sdio_sd.h"
#include"ff.h"

void SD_EraseTest(void);
void SD_SingleBlockTest(void);
void SD_MultiBlockTest(void);
void Fill_Buffer(uint8_t *pBuffer, uint32_t BufferLength, uint32_t Offset);

extern void USART1_Init();

/************************读写测试**********************/
void SDCard_Run(){
    /* 配置中断优先级组 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* 初始化 */
    USART1_Init();
    SD_Error err = SD_Init();
    if(err != SD_OK){
        printf("SD卡初始化失败: %d\r\n", err);
        return;
    }else{
        printf("SD卡初始化成功\r\n");
    }
    
    /* 读写测试 */
    SD_EraseTest();
    SD_SingleBlockTest();
    SD_MultiBlockTest();
}

/************************文件系统测试**********************/
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

void SDCard_Fats_Run(){
    FRESULT err = 0;
    FIL* fp = 0;

    /* 初始化 */
    USART1_Init();

    /* 挂载SD */
    /* FATFS和FIL结构体内存较大，放在堆空间比较合适 */
    FATFS* fs = malloc(sizeof(FATFS));
    err = f_mount(fs, "0:", 1);                 // path为diskio.c中定义的DEV_SDCARD，opt为1表示立即挂载
    
    /* 第一次挂载如果没找到FatFs文件系统，则格式化加载文件系统 */
    if(err == FR_NO_FILESYSTEM){ 
        printf("未加载FatFs系统，需要格式化\r\n");
        uint8_t* work = malloc(512);           // 格式化工作空间
        FATFS_CHECK(f_mkfs("0:", NULL, work, 512));
        free(work);
        FATFS_CHECK(f_mount(fs, "0:", 1));      // 格式化之后重新挂载
    }else if(err != FR_OK){
        fprintf(stderr, "Error line: %d, function: %s, info: %s\r\n", __LINE__, __FUNCTION__, errInfo[err].info);
        goto FATFS_ERR_FLAG;
    }

    /* 打开文件 */
    fp = malloc(sizeof(FIL));
    FATFS_CHECK(f_open(fp, "0:Test.txt", FA_CREATE_ALWAYS | FA_READ | FA_WRITE));

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
    FATFS_CHECK(f_open(fp, "0:Test.txt", FA_READ | FA_OPEN_EXISTING));
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

    /* 查看系统容量大小 */
    DWORD nclst;
    FATFS_CHECK(f_getfree("0:", &nclst, &fs));
    DWORD tot_sect = (fs->n_fatent - 2) * fs->csize;
    DWORD fre_sect = nclst * fs->csize;

    /* Print the free space (assuming 512 bytes/sector) */
    printf("%lu KiB total drive space.\n%lu KiB available.\r\n", tot_sect / 2, fre_sect / 2);
FATFS_ERR_FLAG:
    if(fp) free(fp);
    if(fs) free(fs);
}

/* Private typedef -----------------------------------------------------------*/
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/* Private define ------------------------------------------------------------*/
#define BLOCK_SIZE            512 /* Block Size in Bytes */

#define NUMBER_OF_BLOCKS      32  /* For Multi Blocks operation (Read/Write) */
#define MULTI_BUFFER_SIZE    (BLOCK_SIZE * NUMBER_OF_BLOCKS)

#define SD_OPERATION_ERASE          0
#define SD_OPERATION_BLOCK          1
#define SD_OPERATION_MULTI_BLOCK    2 
#define SD_OPERATION_END            3

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t Buffer_Block_Tx[BLOCK_SIZE], Buffer_Block_Rx[BLOCK_SIZE];
uint8_t Buffer_MultiBlock_Tx[MULTI_BUFFER_SIZE], Buffer_MultiBlock_Rx[MULTI_BUFFER_SIZE];
volatile TestStatus EraseStatus = FAILED, TransferStatus1 = FAILED, TransferStatus2 = FAILED;
SD_Error Status = SD_OK;
__IO uint32_t SDCardOperation = SD_OPERATION_ERASE;

TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint32_t BufferLength);
TestStatus eBuffercmp(uint8_t* pBuffer, uint32_t BufferLength);

/**
  * @brief  Tests the SD card erase operation.
  * @param  None
  * @retval None
  */
void SD_EraseTest(void)
{
  /*------------------- Block Erase ------------------------------------------*/
  if (Status == SD_OK)
  {
    /* Erase NumberOfBlocks Blocks of WRITE_BL_LEN(512 Bytes) */
    Status = SD_Erase(0x00, (BLOCK_SIZE * NUMBER_OF_BLOCKS));
  }

  if (Status == SD_OK)
  {
    Status = SD_ReadMultiBlocks(Buffer_MultiBlock_Rx, 0x00, BLOCK_SIZE, NUMBER_OF_BLOCKS);

    /* Check if the Transfer is finished */
    Status = SD_WaitReadOperation();

    /* Wait until end of DMA transfer */
    while(SD_GetStatus() != SD_TRANSFER_OK);
  }

  /* Check the correctness of erased blocks */
  if (Status == SD_OK)
  {
    EraseStatus = eBuffercmp(Buffer_MultiBlock_Rx, MULTI_BUFFER_SIZE);
  }
  
  if(EraseStatus == PASSED)
  {
    printf("擦除测试成功\r\n");
  }
  else
  {
    printf("擦除测试失败\r\n");
  }
}

/**
  * @brief  Tests the SD card Single Blocks operations.
  * @param  None
  * @retval None
  */
void SD_SingleBlockTest(void)
{
  /*------------------- Block Read/Write --------------------------*/
  /* Fill the buffer to send */
  Fill_Buffer(Buffer_Block_Tx, BLOCK_SIZE, 0x320F);

  if (Status == SD_OK)
  {
    /* Write block of 512 bytes on address 0 */
    Status = SD_WriteBlock(Buffer_Block_Tx, 0x00, BLOCK_SIZE);
    /* Check if the Transfer is finished */
    Status = SD_WaitWriteOperation();
    while(SD_GetStatus() != SD_TRANSFER_OK);
  }

  if (Status == SD_OK)
  {
    /* Read block of 512 bytes from address 0 */
    Status = SD_ReadBlock(Buffer_Block_Rx, 0x00, BLOCK_SIZE);
    /* Check if the Transfer is finished */
    Status = SD_WaitReadOperation();
    while(SD_GetStatus() != SD_TRANSFER_OK);
  }

  /* Check the correctness of written data */
  if (Status == SD_OK)
  {
    TransferStatus1 = Buffercmp(Buffer_Block_Tx, Buffer_Block_Rx, BLOCK_SIZE);
  }
  
  if(TransferStatus1 == PASSED)
  {
    printf("单block写入测试成功\r\n");
  }
  else
  {
    printf("单block写入测试失败\r\n");
  }
}

/**
  * @brief  Tests the SD card Multiple Blocks operations.
  * @param  None
  * @retval None
  */
void SD_MultiBlockTest(void)
{
  /*--------------- Multiple Block Read/Write ---------------------*/
  /* Fill the buffer to send */
  Fill_Buffer(Buffer_MultiBlock_Tx, MULTI_BUFFER_SIZE, 0x0);

  if (Status == SD_OK)
  {
    /* Write multiple block of many bytes on address 0 */
    Status = SD_WriteMultiBlocks(Buffer_MultiBlock_Tx, 0x00, BLOCK_SIZE, NUMBER_OF_BLOCKS);
    /* Check if the Transfer is finished */
    Status = SD_WaitWriteOperation();
    while(SD_GetStatus() != SD_TRANSFER_OK);
  }

  if (Status == SD_OK)
  {
    /* Read block of many bytes from address 0 */
    Status = SD_ReadMultiBlocks(Buffer_MultiBlock_Rx, 0x00, BLOCK_SIZE, NUMBER_OF_BLOCKS);
    /* Check if the Transfer is finished */
    Status = SD_WaitReadOperation();
    while(SD_GetStatus() != SD_TRANSFER_OK);
  }

  /* Check the correctness of written data */
  if (Status == SD_OK)
  {
    TransferStatus2 = Buffercmp(Buffer_MultiBlock_Tx, Buffer_MultiBlock_Rx, MULTI_BUFFER_SIZE);
  }
  
  if(TransferStatus2 == PASSED)
  {
    printf("多block写入测试成功\r\n");
  }
  else
  {
    printf("多block写入测试失败\r\n");
  }
}

/**
  * @brief  Compares two buffers.
  * @param  pBuffer1, pBuffer2: buffers to be compared.
  * @param  BufferLength: buffer's length
  * @retval PASSED: pBuffer1 identical to pBuffer2
  *         FAILED: pBuffer1 differs from pBuffer2
  */
TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint32_t BufferLength)
{
  while (BufferLength--)
  {
    if (*pBuffer1 != *pBuffer2)
    {
      return FAILED;
    }

    pBuffer1++;
    pBuffer2++;
  }

  return PASSED;
}

/**
  * @brief  Fills buffer with user predefined data.
  * @param  pBuffer: pointer on the Buffer to fill
  * @param  BufferLength: size of the buffer to fill
  * @param  Offset: first value to fill on the Buffer
  * @retval None
  */
void Fill_Buffer(uint8_t *pBuffer, uint32_t BufferLength, uint32_t Offset)
{
  uint16_t index = 0;

  /* Put in global buffer same values */
  for (index = 0; index < BufferLength; index++)
  {
    pBuffer[index] = index + Offset;
  }
}

/**
  * @brief  Checks if a buffer has all its values are equal to zero.
  * @param  pBuffer: buffer to be compared.
  * @param  BufferLength: buffer's length
  * @retval PASSED: pBuffer values are zero
  *         FAILED: At least one value from pBuffer buffer is different from zero.
  */
TestStatus eBuffercmp(uint8_t* pBuffer, uint32_t BufferLength)
{
  while (BufferLength--)
  {
    /* In some SD Cards the erased state is 0xFF, in others it's 0x00 */
    if ((*pBuffer != 0xFF) && (*pBuffer != 0x00))
    {
      return FAILED;
    }

    pBuffer++;
  }

  return PASSED;
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif


/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
