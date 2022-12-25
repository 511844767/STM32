/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "exFlash.h"	/* 外部Flash底层函数 */

/* Definitions of physical drive number for each drive */
#define DEV_CARD		0	/* SD卡 */
#define DEV_FLASH_EX	1	/* 外部flash */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;

	switch (pdrv) {
	case DEV_CARD :
		return STA_NOINIT;

	case DEV_FLASH_EX :
		stat = 0;
		bool flag;
		Flash_EX_Errors_t err = Flash_EX_Check_JEDEC_ID(&flag);
		if((!flag) || (err != FLASH_EX_ERR_SUCCESS)){	// ID错误或者函数报错
			stat |= STA_NOINIT;
		}
		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;

	switch (pdrv) {
	case DEV_CARD :
		return STA_NOINIT;

	case DEV_FLASH_EX :
		stat = 0;
		delay_init();						// 延迟
		USART1_Init();  					// 串口
		Flash_EX_Init();					// flash初始化
		Flash_EX_Errors_t err = Flash_EX_Wake_Up();	// 消耗掉第一个异常command
		if(err != FLASH_EX_ERR_SUCCESS){
			stat |= STA_NOINIT;
		}
		stat |= disk_status(DEV_FLASH_EX);	// 检查设备
		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	switch (pdrv) {
	case DEV_CARD :
		return RES_PARERR;

	case DEV_FLASH_EX :
		Flash_EX_Errors_t err = Flash_EX_Read((uint8_t*)buff, sector * 4096, count * 4096);
		switch (err){
		case FLASH_EX_ERR_SUCCESS:
			return RES_OK;		// 成功读取
			break;
		case FLASH_EX_ERR_TIMEOUT:
			return RES_NOTRDY;	// 运行超时
			break;
		case FLASH_EX_OVERFLOWED:
			return RES_ERROR;	// 地址溢出
			break;
		default:
			return RES_PARERR;	// Not allowed
		}
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	switch (pdrv) {
	case DEV_CARD :
		return RES_PARERR;

	case DEV_FLASH_EX :
		Flash_EX_Errors_t err = Flash_EX_Write((uint8_t*)buff, sector * 4096, count * 4096);
		switch (err){
		case FLASH_EX_ERR_SUCCESS:
			return RES_OK;		// 成功读取
			break;
		case FLASH_EX_ERR_TIMEOUT:
			return RES_NOTRDY;	// 运行超时
			break;
		case FLASH_EX_OVERFLOWED:
			return RES_ERROR;	// 地址溢出
			break;
		default:
			return RES_PARERR;	// Not allowed
		}
	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions获取一些芯片信息                                */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_OK;

	switch (pdrv) {
	case DEV_CARD :
		return res;

	case DEV_FLASH_EX :
		switch (cmd)
		{
		case GET_SECTOR_COUNT:	// 扇区个数
			*(LBA_t*)buff = 4096;
			break;
		case GET_SECTOR_SIZE:	// 扇区大小
			*(WORD*)buff = 4096;
			break;
		case GET_BLOCK_SIZE:	// FatFs block的含义是最小擦除粒度，而Flash的block含义是16个sector
			*(DWORD*)buff = 1;
			break;
		default:
			break;
		}
		return res;
	}

	return RES_PARERR;
}

/**
 * @brief 	获取文件时间
 * @note 	STM32暂时无法获取时间，返回0
 * TODO: 如果需要获取实时时间，需要参考教程RTC章节
 */
DWORD get_fattime (void){
	return 0;
}