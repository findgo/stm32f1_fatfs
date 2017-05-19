
#ifndef __SD_H_
#define __SD_H_

#include "app_cfg.h"
#include "hal_spi.h"
#include "diskio.h"



// SD�����Ͷ���  
#define SD_TYPE_MMC     0
#define SD_TYPE_V1      1
#define SD_TYPE_V2      2
#define SD_TYPE_V2HC    4	  

#define ACMD_IDENT 0x80 // ����ʶ�� ACMDָ��

// SD��ָ���  	   
#define CMD0    0       //GO_IDLE_STATE ����λ
#define CMD1    1
#define	CMD8	8		// 
#define CMD9    9       //����9 ����CSD����
#define CMD10   10      //����10����CID����
#define CMD12   12      //����12��ֹͣ���ݴ���
#define CMD16   16      //����16������SectorSize Ӧ����0x00
#define CMD17   17      //����17����sector
#define CMD18   18      //����18����Multi sector
#define CMD24   24      //����24��дsector
#define CMD25   25      //����25��дMulti sector
#define CMD32   32
#define CMD33   33
#define CMD38   38
#define CMD55   55      //����55��Ӧ����0x01
#define CMD58   58      //����58����OCR��Ϣ
#define CMD59   59      //����59��ʹ��/��ֹCRC��Ӧ����0x00

#define ACMD13  (ACMD_IDENT + 13)
#define ACMD23  (ACMD_IDENT + 23)      //����23�����ö�sectorд��ǰԤ�Ȳ���N��block
#define ACMD41  (ACMD_IDENT + 41)      //����41��Ӧ����0x00


/*** Ӳ���ӿڶ���******************************************************************************/
// SPI
#define		SD_SPI_ReadWrite(dat)		SPI1_ReadWrite(dat)						
#define		SD_SPI_InitLowBaud()		SPI1_Init(SPI_BaudRatePrescaler_256)	// ��ʼ���ɵ��ٶ�ģʽ
#define		SD_SPI_InitHighBaud_Read()	SPI1_Init(SPI_BaudRatePrescaler_8)	// ������ 64/8=8M
#define		SD_SPI_InitHighBaud_Write()	SPI1_Init(SPI_BaudRatePrescaler_32)	// д���� 64/32=2M
#define		SD_SPI_InitHighBaud()		SD_SPI_InitHighBaud_Write()				// Ĭ���Զ�ȡ������

// CSƬѡ
#define		SD_CS_PIN		PAout(4)
#define		SD_CSL()		SD_CS_PIN = 0
#define		SD_CSH()		SD_CS_PIN = 1

// PWR ��Դʹ��
#define		SD_PWR_PIN		PAout(0)
#define		SD_PWR_EN()		SD_PWR_PIN = 0
#define		SD_PWR_DIS()	SD_PWR_PIN = 1

// ����Ƿ����  1:���� 0: ����
#define		SD_IsPlugIn()	PCin(13) == 0   // �͵�ƽΪ���룬�ߵ�ƽΪ����
// ����Ƿ�д����  1: д����  0:δд����
#define     SD_IsWriteEnable() 0
/*** ����ӿ�**************************************************************************************/			    	 
/*
 * ��� sd���Ƿ����
 * 1:  ����
 * 0:  ����
*/
uint8_t SD_DetectPugIn(void);
void SD_PortInit( void );
DSTATUS SD_Init(void);  // SD����ʼ��
/*
    sd����ǰ������״̬
*/
DSTATUS SD_status(void);
uint8_t SD_GetSDType(void);    // ���sd������
DRESULT SD_ReadMultiBlock(uint32_t sector, uint8_t *buf, uint8_t cnt); 		// �����BLOCK
DRESULT SD_WriteMultiBlock(uint32_t sector,const  uint8_t *buf, uint8_t cnt);	// д���BLOCK
DRESULT SD_Ioctl(uint8_t cmd,uint8_t *buff);

#define MMC_disk_status()     SD_status()
#define MMC_disk_initialize() SD_Init()
#define MMC_disk_read(buff, sector, count) SD_ReadMultiBlock(sector,buff, count)
#define MMC_disk_write(buff, sector, count) SD_WriteMultiBlock(sector,buff, count)
#define MMC_disk_ioctl(cmd,buff) SD_Ioctl(cmd,buff)

#endif

