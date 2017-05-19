
#ifndef __SD_H_
#define __SD_H_

#include "app_cfg.h"
#include "hal_spi.h"
#include "diskio.h"



// SD卡类型定义  
#define SD_TYPE_MMC     0
#define SD_TYPE_V1      1
#define SD_TYPE_V2      2
#define SD_TYPE_V2HC    4	  

#define ACMD_IDENT 0x80 // 用于识别 ACMD指令

// SD卡指令表  	   
#define CMD0    0       //GO_IDLE_STATE 卡复位
#define CMD1    1
#define	CMD8	8		// 
#define CMD9    9       //命令9 ，读CSD数据
#define CMD10   10      //命令10，读CID数据
#define CMD12   12      //命令12，停止数据传输
#define CMD16   16      //命令16，设置SectorSize 应返回0x00
#define CMD17   17      //命令17，读sector
#define CMD18   18      //命令18，读Multi sector
#define CMD24   24      //命令24，写sector
#define CMD25   25      //命令25，写Multi sector
#define CMD32   32
#define CMD33   33
#define CMD38   38
#define CMD55   55      //命令55，应返回0x01
#define CMD58   58      //命令58，读OCR信息
#define CMD59   59      //命令59，使能/禁止CRC，应返回0x00

#define ACMD13  (ACMD_IDENT + 13)
#define ACMD23  (ACMD_IDENT + 23)      //命令23，设置多sector写入前预先擦除N个block
#define ACMD41  (ACMD_IDENT + 41)      //命令41，应返回0x00


/*** 硬件接口定义******************************************************************************/
// SPI
#define		SD_SPI_ReadWrite(dat)		SPI1_ReadWrite(dat)						
#define		SD_SPI_InitLowBaud()		SPI1_Init(SPI_BaudRatePrescaler_256)	// 初始化成低速度模式
#define		SD_SPI_InitHighBaud_Read()	SPI1_Init(SPI_BaudRatePrescaler_8)	// 读速率 64/8=8M
#define		SD_SPI_InitHighBaud_Write()	SPI1_Init(SPI_BaudRatePrescaler_32)	// 写速率 64/32=2M
#define		SD_SPI_InitHighBaud()		SD_SPI_InitHighBaud_Write()				// 默认以读取的速率

// CS片选
#define		SD_CS_PIN		PAout(4)
#define		SD_CSL()		SD_CS_PIN = 0
#define		SD_CSH()		SD_CS_PIN = 1

// PWR 电源使能
#define		SD_PWR_PIN		PAout(0)
#define		SD_PWR_EN()		SD_PWR_PIN = 0
#define		SD_PWR_DIS()	SD_PWR_PIN = 1

// 检查是否插入  1:插入 0: 弹出
#define		SD_IsPlugIn()	PCin(13) == 0   // 低电平为插入，高电平为弹出
// 检查是否写保护  1: 写保护  0:未写保护
#define     SD_IsWriteEnable() 0
/*** 对外接口**************************************************************************************/			    	 
/*
 * 检测 sd卡是否插入
 * 1:  插入
 * 0:  弹出
*/
uint8_t SD_DetectPugIn(void);
void SD_PortInit( void );
DSTATUS SD_Init(void);  // SD卡初始化
/*
    sd卡当前所处的状态
*/
DSTATUS SD_status(void);
uint8_t SD_GetSDType(void);    // 获得sd卡类型
DRESULT SD_ReadMultiBlock(uint32_t sector, uint8_t *buf, uint8_t cnt); 		// 读多个BLOCK
DRESULT SD_WriteMultiBlock(uint32_t sector,const  uint8_t *buf, uint8_t cnt);	// 写多个BLOCK
DRESULT SD_Ioctl(uint8_t cmd,uint8_t *buff);

#define MMC_disk_status()     SD_status()
#define MMC_disk_initialize() SD_Init()
#define MMC_disk_read(buff, sector, count) SD_ReadMultiBlock(sector,buff, count)
#define MMC_disk_write(buff, sector, count) SD_WriteMultiBlock(sector,buff, count)
#define MMC_disk_ioctl(cmd,buff) SD_Ioctl(cmd,buff)

#endif

