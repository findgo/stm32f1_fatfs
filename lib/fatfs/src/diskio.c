/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "SD.h"

/* Definitions of physical drive number for each drive */
#define DEV_MMC		0	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_RAM		1	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */
#define DEV_FLASH   3

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
//	int result;

	switch (pdrv) {

	case DEV_MMC :
		stat = MMC_disk_status();
		// translate the reslut code here

		return stat;
        
    case DEV_RAM :
//      stat = RAM_disk_status();
        stat = STA_OK;
        // translate the reslut code here

        return stat;

	case DEV_USB :
//		stat = USB_disk_status();
        stat = STA_OK;
		// translate the reslut code here

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
//	int result;

	switch (pdrv) {

	case DEV_MMC :
		stat = MMC_disk_initialize();        
		// translate the reslut code here

		return stat;
        
    case DEV_RAM :
//      stat = RAM_disk_initialize();
        stat = STA_OK;
        // translate the reslut code here

        return stat;

	case DEV_USB :
//		stat = USB_disk_initialize();
        stat = STA_OK;
		// translate the reslut code here

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
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
//	int result;

	switch (pdrv) {
	case DEV_MMC :
		// translate the arguments here

		res = MMC_disk_read(buff, sector, count);
		// translate the reslut code here

		return res;
    case DEV_RAM :
        // translate the arguments here

//      res = RAM_disk_read(buff, sector, count);
        res = RES_OK;
        // translate the reslut code here

        return res;

	case DEV_USB :
		// translate the arguments here

//		res = USB_disk_read(buff, sector, count);
        res = RES_OK;
		// translate the reslut code here

		return res;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
//	int result;

	switch (pdrv) {
	case DEV_MMC :
		// translate the arguments here

		res = MMC_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;
    case DEV_RAM :
        // translate the arguments here

//      res = RAM_disk_write(buff, sector, count);
        res = RES_OK;
        // translate the reslut code here

        return res;

	case DEV_USB :
		// translate the arguments here

//		res = USB_disk_write(buff, sector, count);
        res = RES_OK;
		// translate the reslut code here

		return res;
	}

	return RES_PARERR;
}


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
//	int result;

	switch (pdrv) {
	case DEV_MMC :

		// Process of the command for the MMC/SD card
        res = MMC_disk_ioctl(cmd,buff);
        
		return res;
    case DEV_RAM :
    
        // Process of the command for the RAM drive
        res = RES_OK;
        return res;

	case DEV_USB :

		// Process of the command the USB drive
        res = RES_OK;
		return res;
	}

	return RES_PARERR;
}

// user add declar in ff.h
DWORD get_fattime (void)
{
    return 0;
}

