/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "spi_flash.h"
#include "common.h"

/* Definitions of physical drive number for each drive */
#define SPI		0	/* Example: Map SPI drive to drive number 0 */
#define MMC		1	/* Example: Map MMC/SD card to drive number 1 */
#define USB		2	/* Example: Map USB drive to drive number 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{

	switch (pdrv) {
	case SPI :
		return 0;

	default:
		return STA_NODISK;
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
	case SPI :
		sFLASH_Init();
		DBG_MSG("sFLASH_ReadID()=0x%x", sFLASH_ReadID());

		return 0;

	default:
		return STA_NODISK;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{


	switch (pdrv) {
	case SPI :
		sFLASH_ReadBuffer(buff, sector*sFLASH_SPI_SECTORSIZE, count*sFLASH_SPI_SECTORSIZE);

		return RES_OK;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	int i;


	switch (pdrv) {
	case SPI :
		for (i = 0; i < count; ++i)
		{
			sFLASH_EraseSector((sector + i)*sFLASH_SPI_SECTORSIZE);
		}
		sFLASH_WriteBuffer(buff, sector*sFLASH_SPI_SECTORSIZE, count*sFLASH_SPI_SECTORSIZE);

		return RES_OK;
	}

	return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_PARERR;

	switch (pdrv) {
	case SPI :

		switch(cmd){
		case CTRL_SYNC:
			res = RES_OK;
			break;
		case GET_SECTOR_COUNT:
			*(DWORD*)buff = sFLASH_SPI_SECTORCOUNT;
			res = RES_OK;
			break;
		case GET_SECTOR_SIZE:
			*(WORD*)buff = sFLASH_SPI_SECTORSIZE;
			res = RES_OK;
			break;
		case GET_BLOCK_SIZE:
			*(WORD*)buff = sFLASH_SPI_SECTORSIZE;
			res = RES_OK;
			break;
		}

		return res;

	}

	return RES_PARERR;
}
#endif

DWORD get_fattime (void)
{
	return 0;
}

