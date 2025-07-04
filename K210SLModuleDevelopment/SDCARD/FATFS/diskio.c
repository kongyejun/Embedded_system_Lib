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
#include "sdcard.h"		/*添加底层SD驱动头文件*/

#ifndef __weak
#define __weak __attribute__((weak))
#endif

/* 源码中定义的，就是将不同的设备定义为不同的编号，
告诉底层要操作的介质是哪一个，以便后续函数进行根据编号的不同执行相应的函数。*/
// #define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
// #define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
// #define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */

/*FatFs是支持多物理设备的，必须为每个物理设备定义一个不同的编号;一下是用户自己定义的*/
#define SD_CARD		0

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* 设备物理编号(用户自定义的) */
){
	//DSTATUS stat;
	//int result;
	switch (pdrv) {
	case SD_CARD:
		return 0;
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
	switch (pdrv) {
	case SD_CARD :
		if(sd_init() == 0){return 0;}
		break;
	default :break;	
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
	case SD_CARD :
		if(sd_read_sector(buff,sector,count)==0)return RES_OK;
		break;
	default:break;
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
	case SD_CARD:
		if(sd_write_sector((BYTE *)buff,sector,count)==0)return RES_OK;
		break;
	default : break;
	}
	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
//调用disk_ioctl函数以控制设备的特定功能和除通用读/写之外的其他功能
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_ERROR;
	switch (cmd) {
	/*  确保设备已完成挂起的写入过程(因为在底层实现时,都会清空输入缓存,所以可以直接跳过)  */
	case CTRL_SYNC:
		res = RES_OK;
		break;
	/* 获取扇区个数 */
	case GET_SECTOR_COUNT:
		*(DWORD *)buff = (cardinfo.SD_csd.DeviceSize + 1) << 10;
		res = RES_OK;
		break;
	/* 获取扇区大小 */
	case GET_SECTOR_SIZE:
		*(WORD *)buff = cardinfo.CardBlockSize;
		res = RES_OK;
		break;
	/* 获取擦除扇区最小个数 */
	case GET_BLOCK_SIZE:
		*(DWORD *)buff = cardinfo.CardBlockSize;
		res = RES_OK;
		break;
	default:
		res = RES_PARERR;
	}
	return res;
}

__weak DWORD get_fattime(void) {
	/* 返回当前时间戳 */
	return	  ((DWORD)(2015 - 1980) << 25)	/* Year 2015 */
			| ((DWORD)1 << 21)				/* Month 1 */
			| ((DWORD)1 << 16)				/* Mday 1 */
			| ((DWORD)0 << 11)				/* Hour 0 */
			| ((DWORD)0 << 5)				  /* Min 0 */
			| ((DWORD)0 >> 1);				/* Sec 0 */
}