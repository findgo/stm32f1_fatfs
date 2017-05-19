

#include "SD.h"

// for spi r1 (0b0xxxxxxx)
#define SD_R1_IDLE          0x01
#define SD_R1_ERASE_RESET   0x02
#define SD_R1_ILLEGAL_CMD   0x04
#define SD_R1_CRC_ERROR     0x08
#define SD_R1_ERASE_SEQERR  0x10
#define SD_R1_ADDRESS_ERR   0x20
#define SD_R1_PARA_ERR      0x40
#define SD_R1_RESERVED      0x80


/*** ���ر�������*******************************************************************************/
static uint8_t SD_Type = 0;//SD��������
static DSTATUS SD_Stat = STA_NOINIT;
/*
 * �ɹ�: 0
 * ʧ��: other
 */
static uint8_t __SD_WaitReady(void)
{
    uint16_t t = 0;
    
    do{
        if(SD_SPI_ReadWrite(0xFF) == 0xFF)
            return 0;//OK
    }while(++t < 0xFFFE);
    
    return 1;
}


//�ͷ�SPI����
static void __SD_DisSelect(void)
{
    SD_CSH();
    SD_SPI_ReadWrite(0xff);//�ṩ�����8��ʱ��
}
//ѡ��sd��,���ҵȴ���׼��OK
//����ֵ:0,�ɹ�;1,ʧ��;
static uint8_t __SD_Select(void)
{
    SD_CSL();
    if(__SD_WaitReady() == 0)
        return 0;//�ȴ��ɹ�
    __SD_DisSelect();
    
    return 1;//�ȴ�ʧ��
}
/*
 * ��� sd���Ƿ����
 * 1:  ����
 * 0:  ����
*/
uint8_t SD_DetectPugIn(void)
{
    return SD_IsPlugIn();
}
/*
 * ��� sd���Ƿ�д����
 * 1:  ����
 * 0:  δ����
*/
static uint8_t SD_DetectWriteProtectEnabled(void)
{
    return SD_IsWriteEnable();
}
/********************************************************************************************************
** ����: SD_SendCmd, �ȴ�SD����Ӧ
**------------------------------------------------------------------------------------------------------
** ����: cmd    :   ����
**       arg    :   �������
** ����: SD�����ص���Ӧ 
** NOTE: 
********************************************************************************************************/
static uint8_t SD_SendCmd(uint8_t cmd, uint32_t arg)
{
    uint8_t retry_crc;  
    uint8_t res; 

 	if (cmd & ACMD_IDENT) {	/* Send a CMD55 prior to ACMD<n> */
		cmd &= 0x7F;
		res = SD_SendCmd(CMD55, 0);
		if (res > 1) 
            return res;
	}   
    
    if(cmd != CMD12){
        __SD_DisSelect();
        if(__SD_Select() )
            return 0xFF;//select failed
    }
    SD_SPI_ReadWrite(cmd | 0x40);
    SD_SPI_ReadWrite(arg >> 24);
    SD_SPI_ReadWrite(arg >> 16);
    SD_SPI_ReadWrite(arg >> 8);
    SD_SPI_ReadWrite(arg); 

    if(cmd == CMD0)
        retry_crc = 0x95;   /* Valid CRC for CMD0(0) */
    else if(cmd == CMD8)
        retry_crc = 0x87;   /* Valid CRC for CMD8(0x1AA) */
    else 
        retry_crc = 0x01;   
    SD_SPI_ReadWrite(retry_crc); // write crc
    
    if(cmd == CMD12)
        SD_SPI_ReadWrite(0xff);//Skip a stuff byte when stop reading
    //wait for response,timeout exit
    retry_crc = 32;
    do{
        res = SD_SPI_ReadWrite(0xff);
    }while((res & 0x80) && --retry_crc);    

    return res;
}                                                                                       


/********************************************************************************************************
** ����: SD_PortInit, SD��Ӳ���ӿڳ�ʼ��
**------------------------------------------------------------------------------------------------------
** ����: ��
** ����: ��
** NOTE: 
********************************************************************************************************/
void SD_PortInit( void )
{   
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    // ����IO 
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC,ENABLE);          // �򿪶�ӦIO������ʱ��
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_4;        // PA0--PWR_EN PA4--CS
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;                    // ��������,����Ϊ�͡�����Ϊ��
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    PWR_BackupAccessCmd(ENABLE);                                    // �����޸�RTC
    RCC_LSEConfig(RCC_LSE_OFF);                                     // �ر��ⲿ����ʱ���źŹ���,����������PC13~PC15     

    // ���ÿ��������ж�
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;                // �ⲿ�ж�2
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;       //��ռ���ȼ�
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2 ;                    //�����ȼ�  
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                     // ʹ��
    NVIC_Init(&NVIC_InitStructure);
    
    //��������AFIO�ⲿ�ж����üĴ���AFIO_EXTICR1������ѡ��EXTI13�ⲿ�жϵ�����Դ��PC1��
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource13);        // �ⲿ�ж�����AFIO--ETXI1
    EXTI_InitStructure.EXTI_Line    = EXTI_Line13;                      // PC1 ��Ϊ����K3 ���״̬
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;              // �ж�ģʽ
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;      // ˫���ش���
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure); 

    // SD��������ʼ��
    SD_SPI_InitLowBaud();   
    SD_CSH();
    SD_PWR_EN();
}


/********************************************************************************************************
** ����: SD_Init, ��ʼ��SD��
**------------------------------------------------------------------------------------------------------
** ����: ��
** ����: 0      :   ��ʼ���ɹ�
**       ����   :   �μ�SD������ֵ
** NOTE: 
********************************************************************************************************/
DSTATUS SD_Init(void)
{ 
    uint8_t r1,i;      
    uint8_t buf[4]; 
    uint16_t retry;
 
    SD_Type = 0;//Ĭ���޿�

    // ��ʼ��SD����SPIģʽ д������ ������
    SD_SPI_InitLowBaud();   

    if(!SD_DetectPugIn())
        return STA_NODISK;

    // ��ʶ��ģʽ Ĭ��ʶ��ʱ��Ƶ�� < 400KHZ
    SD_CSH();
    for(i = 0;i < 12; i++)
        SD_SPI_ReadWrite(0xff);//��������74��������
    retry = 200;
    do{
        r1 = SD_SendCmd(CMD0,0);// send COM0 go to idle
    }while((r1 != 0x01) && --retry);
    // ����spiģʽʧ��
    if(retry == 0){
        r1 = 1;
        goto EXIT_FUNC; 
    }
    
    // get SD version message
    r1 = SD_SendCmd(CMD8,0x1AA);
    if(r1 == 0x05){ //SD V1.x/ MMC V3        
        __SD_DisSelect();
    
        if(SD_SendCmd(ACMD41,0) <= 1){       
            SD_Type = CT_SD1;
            retry = 1400;
            do {//�ȴ��˳�IDLEģʽ                
                r1 = SD_SendCmd(ACMD41,0);//����CMD41
            }while(r1 && --retry);
        }else{
            SD_Type = CT_MMC;//MMC V3
            retry = 400;
            do {//�ȴ��˳�IDLEģʽ
                r1 = SD_SendCmd(CMD1,0);//����CMD1
            }while(r1 && --retry);  
        }
        
       retry = 100;
       do {//�ȴ��˳�IDLEģʽ
           r1 = SD_SendCmd(CMD16, 512);//����sector size Ϊ512
       }while(r1 && --retry);
       if(retry == 0)
           SD_Type = 0;
    }else if(r1 == 0x01){ //SD V2.0
        for(i = 0; i < 4; i++)
            buf[i] = SD_SPI_ReadWrite(0xff);  //Get trailing return value of R7 resp
        
        if(!(buf[2] == 0x01 && buf[3] == 0xAA)) {//���Ƿ�֧��2.7~3.6V
            r1 = 1;
            goto EXIT_FUNC;
        }
        
        retry = 400;
        do{
            r1 = SD_SendCmd(ACMD41,0x40000000);//����CMD41
        }while(r1 && --retry);

        //����SD2.0���汾��ʼ
        if(retry && SD_SendCmd(CMD58,0) == 0){ /* Check CCS bit in the OCR */
            for(i = 0; i < 4; i++)
                buf[i] = SD_SPI_ReadWrite(0xFF);//�õ�OCRֵ
            if(buf[0] & 0x40)
                SD_Type = CT_SD2 | CT_BLOCK;    //���CCS
            else 
                SD_Type = CT_SD2;   
        }
    }
	
EXIT_FUNC:
    __SD_DisSelect();
    SD_SPI_InitHighBaud();//����
    if (r1) {			//failed
		SD_Stat |= STA_NOINIT;	/* set STA_NOINIT flag */
	} else {			//ok
		SD_Stat &= ~STA_NOINIT; /* clear STA_NOINIT flag  */
	}

	if (!SD_DetectWriteProtectEnabled()) {
		SD_Stat |= STA_PROTECT;
	} else {
		SD_Stat &= ~STA_PROTECT;
	}
    
    if(r1)
        SD_PWR_DIS();//�����ʼ��ʧ�ܣ��رտ��ĵ�Դ

    return r1;  
}

DSTATUS SD_status(void)
{
    if(!SD_DetectPugIn())
		return STA_NOINIT;

	/* Check if write is enabled */
	if (SD_DetectWriteProtectEnabled()) {
		SD_Stat |= STA_PROTECT;
	} else {
		SD_Stat &= ~STA_PROTECT;
	}
	    
    return SD_Stat;
}

/********************************************************************************************************
** ����: __SD_ReceiveData, ��SD���ж���ָ�����ȵ����ݣ������ڸ���λ��
**------------------------------------------------------------------------------------------------------
** ����: data   :   ��ȡ���ݵĴ�Ŵ�
**       len    :   ���ݳ���
** ����: 0      :   �ɹ�
**       ����   :   ���� �μ�SD������ֵ
** NOTE: 
********************************************************************************************************/
static uint8_t __SD_ReceiveData(uint8_t *data, uint16_t len)
{ 
    uint16_t retry = 510;//�ȴ�����
    
    while ((SD_SPI_ReadWrite(0xFF) != 0xFE) && --retry);//�ȴ��õ�׼ȷ�Ļ�Ӧ  
  
    if (retry == 0)
        return 1;//�õ���Ӧʧ��   

    while(len--)//��ʼ��������
    {
        *data = SD_SPI_ReadWrite(0xFF);
        data++;
    }
    //������2��αCRC��dummy CRC��
    SD_SPI_ReadWrite(0xFF);
    SD_SPI_ReadWrite(0xFF);  
    
    return 0;//��ȡ�ɹ�
}           

//���SD������
uint8_t SD_GetSDType(void)
{
    return SD_Type;
}
/********************************************************************************************************
** ����: SD_GetCID, ��ȡSD����CID��Ϣ��������������Ϣ
**------------------------------------------------------------------------------------------------------
** ����: cid_data   :   (���CID���ڴ棬16Byte�����
** ����: 0          :   ��ʼ���ɹ�
**       ����       :   ���� �μ�SD������ֵ
** NOTE: ��ϸ�ṹ�μ�==http://blog.csdn.net/lwj103862095/article/details/38335709
********************************************************************************************************/
uint8_t SD_GetCID( uint8_t *cid_data )
{
    uint8_t r1;   
    
    //��CMD10�����CID
    if(SD_SendCmd(CMD10,0) == 0x00){
        r1 = __SD_ReceiveData(cid_data,16);//����16���ֽڵ�����  
    }
    __SD_DisSelect();//ȡ��Ƭѡ
    
    if(r1)
        return 1;

    return 0;
}           

/********************************************************************************************************
** ����: SD_GetCID, ��ȡSD����CSD��Ϣ�������������ٶ���Ϣ
**------------------------------------------------------------------------------------------------------
** ����: csd_data   :   (���CSD���ڴ棬16Byte�����
** ����: 0          :   ��ʼ���ɹ�
**       ����       :   ���� �μ�SD������ֵ
** NOTE: ��ϸ�ṹ�μ�==http://blog.csdn.net/lwj103862095/article/details/38335709
********************************************************************************************************/
uint8_t SD_GetCSD(uint8_t *csd_data)
{
    uint8_t r1;
    
    if(SD_SendCmd(CMD9,0) == 0){    //��CMD9�����CSD
        r1 = __SD_ReceiveData(csd_data, 16);//����16���ֽڵ����� 
    }
    __SD_DisSelect();//ȡ��Ƭѡ
    
    if(r1)
        return 1;
    else 
        return 0;
}  

/********************************************************************************************************
** ����: SD_GetByteCap, ��ȡSD��������Ϊ��λ��������С   
**------------------------------------------------------------------------------------------------------
** ����:  :   SD����
** NOTE: 
********************************************************************************************************/
uint32_t SD_GetSectorCount(void)
{
    uint8_t n;
    uint8_t csd[16];
    uint16_t csize;
    uint32_t Capacity;
    
    //ȡCSD��Ϣ������ڼ��������0
    if(SD_GetCSD(csd) != 0) 
        return 0; 
    
    //���ΪSDHC�����������淽ʽ����
    if((csd[0] & 0xC0) == 0x40){  /* SDC ver 2.00 */
		csize = csd[9] + ((uint16_t)csd[8] << 8) + ((uint16_t)(csd[7] & 63) << 16) + 1;
        Capacity = (uint32_t)csize << 10;//�õ�������               
    }else{      /* SDC ver 1.XX or MMC ver 3 */
        n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
        csize = (csd[8] >> 6) + ((uint16_t)csd[7] << 2) + ((uint16_t)(csd[6] & 3) << 10) + 1;
        Capacity= (uint32_t)csize << (n - 9);//�õ�������   
    }
    return Capacity;
}

/********************************************************************************************************
** ����: SD_ReadMultiBlock, ��SD���Ķ��block(ʵ�ʲ��Թ�)   
**------------------------------------------------------------------------------------------------------
** ����: sector :   ȡ��ַ��sectorֵ���������ַ�� 
**       buffer :   ���ݴ洢��ַ����С����512byte�� 
**       blockcnt  :   ������count��block  
** ����: 0      :   �ɹ�
**       ����   :   ʧ��
** NOTE: 
********************************************************************************************************/
DRESULT SD_ReadMultiBlock(uint32_t sector, uint8_t *buf, uint8_t blockcnt)
{
    uint8_t cmd;

    if(!SD_DetectPugIn() || (SD_Stat & STA_NOINIT)){
        return RES_NOTRDY;
    }

    SD_SPI_InitHighBaud_Read();
    
    if(!(SD_Type & CT_BLOCK)) /* LBA ==> BA conversion (byte addressing cards) */
        sector <<= 9;//ת��Ϊ�ֽڵ�ַ

    cmd = (blockcnt == 1) ? CMD17 : CMD18; 
    
    if(SD_SendCmd(cmd,sector) != 0x00){
        __SD_DisSelect();//ȡ��Ƭѡ
        return RES_ERROR;
     }
    
    do{
        if(__SD_ReceiveData(buf,512) != 0x00) 
            break;      // ��������
        buf += 512;
        --blockcnt;
    }while(blockcnt);
    
    if(cmd == CMD18){
        (void)SD_SendCmd(CMD12,0);   //���� ����ֹͣ����
    }
    __SD_DisSelect();//ȡ��Ƭѡ
   
	return blockcnt ? RES_ERROR : RES_OK;	/* Return result */
}                                             
//��sd��д��һ�����ݰ������� 512�ֽ�
//buf:���ݻ�����
//cmd:ָ��
//����ֵ:0,�ɹ�; >0 ʧ��;  
static uint8_t __SD_SendBlock(const uint8_t *buf,uint8_t cmd)
{
    uint16_t i;
        
    // ��������ʱ�Ӻ�ʼд
    SD_SPI_ReadWrite(0xff);
    SD_SPI_ReadWrite(0xff);
    SD_SPI_ReadWrite(0xff);

    SD_SPI_ReadWrite(cmd);  
    for(i = 0;i < 512; i++)
        SD_SPI_ReadWrite(buf[i]);
    // �������� dummy CRC
    SD_SPI_ReadWrite(0xFF);//����crc
    SD_SPI_ReadWrite(0xFF);
    
    return __SD_WaitReady();
}
/********************************************************************************************************
** ����: SD_WriteMultiBlock,д��SD����N��block(δʵ�ʲ��Թ�)                                
**------------------------------------------------------------------------------------------------------
** ����: sector :   ��ַ��sectorֵ���������ַ�� 
**       data   :   д���512�ֽ�* blockcnt
**       blockcnt    :   д���block��Ŀ
** ����: 0      :   �ɹ�
**       ����   :   ���� �μ�SD������ֵ
** NOTE: 
********************************************************************************************************/
DRESULT SD_WriteMultiBlock(uint32_t sector, const uint8_t *buf, uint8_t blockcnt )
{
    uint8_t cmd;

	if (!SD_DetectPugIn()) {
		return RES_ERROR;
	}
	if (SD_DetectWriteProtectEnabled()) {
		return RES_WRPRT;
	}
	if (SD_Stat & STA_NOINIT) {
		return RES_NOTRDY;	/* Check drive status */
	}
	if (SD_Stat & STA_PROTECT) {
		return RES_WRPRT;	/* Check write protect */
	}
    
    SD_SPI_InitHighBaud_Write();

    if(!(SD_Type & CT_BLOCK)) /* LBA ==> BA conversion (byte addressing cards) */
        sector <<= 9;//ת��Ϊ�ֽڵ�ַ

    if(blockcnt == 1)
        cmd = CMD24;
    else{
        cmd = CMD25;
        /*
        // ���Ŀ�꿨����MMC��,����ACMD23ָ��ʹ��Ԥ����
        if(SD_Type != SD_TYPE_MMC){
            SD_SendCmd(CMD55,0,0x01);   
            SD_SendCmd(ACMD23,cnt,0X01);//����ָ��  
         }  
        */
    }
    if(SD_SendCmd(cmd,sector) != 0x00){
        __SD_DisSelect();
        return RES_ERROR;
     }
    
    if(blockcnt == 1){ // д������
        if(__SD_SendBlock(buf,0xFE) == 0)//д512���ֽ� 
            blockcnt = 0;
    }else{ // д������
        do{
            if(__SD_SendBlock(buf,0xFC))//����512���ֽ�  
                break;
            buf += 512;
            --blockcnt;
        }while(blockcnt);
        (void)SD_SPI_ReadWrite(0xFD);//���ͽ�������
        (void)__SD_WaitReady();
    }   
    __SD_DisSelect();
    
	return blockcnt ? RES_ERROR : RES_OK;	/* Return result */
}  
DRESULT SD_Ioctl(uint8_t cmd,uint8_t *buff)
{
	DRESULT res;
	DWORD *dp, st, ed, csize;
    BYTE n,csd[16];
    
	if (SD_Stat & STA_NOINIT) {
		return RES_NOTRDY;	/* Check if drive is ready */
	}
	if (!SD_DetectPugIn()) {
		return RES_NOTRDY;
	}

	res = RES_ERROR;
    switch(cmd){
        case CTRL_SYNC:
            if(!__SD_Select()){
                res = RES_OK;
             }
            __SD_Select();
            break;
        case GET_SECTOR_COUNT:
            csize = SD_GetSectorCount();
            if(csize){
                *(uint32_t *)buff  = csize;
			    res = RES_OK;
            }
            break;
        case GET_SECTOR_SIZE:
            *(uint32_t *)buff = 512;
            break;
        case GET_BLOCK_SIZE:
		if (SD_Type & CT_SD2) {	/* SDC ver 2.00 */
			if (SD_SendCmd(ACMD13, 0) == 0) {	/* Read SD status */
				SD_SPI_ReadWrite(0xFF);
				if (__SD_ReceiveData(csd, 16) == 0) {				/* Read partial block */
					for (n = 64 - 16; n; n--) 
                        SD_SPI_ReadWrite(0xFF); /* Purge trailing data */
					*(DWORD*)buff = 16UL << (csd[10] >> 4);
					res = RES_OK;
				}
			}
		} else {					/* SDC ver 1.XX or MMC */
			if ((SD_SendCmd(CMD9, 0) == 0) && (__SD_ReceiveData(csd, 16) == 0) ) {	/* Read CSD */
				if (SD_Type & CT_SD1) {	/* SDC ver 1.XX */
					*(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
				} else {					/* MMC */
					*(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
				}
				res = RES_OK;
			}
		}            
            break;
        case CTRL_TRIM:	/* Erase a block of sectors (used when _USE_ERASE == 1) */
    		if (!(SD_Type & CT_SDC))				/* Check if the card is SDC */
                break;
            if (SD_Ioctl(MMC_GET_CSD, csd)) /* Get CSD */
                break;  
    		if (!(csd[0] >> 6) && !(csd[10] & 0x40)) 	/* Check if sector erase can be applied to the card */
                break;
            dp = (DWORD *)buff; 
            st = dp[0]; 
            ed = dp[1];				/* Load sector block */
    		if (!(SD_Type & CT_BLOCK)) {
    			st *= 512; 
                ed *= 512;
    		}
    		if (SD_SendCmd(CMD32, st) == 0 
                && SD_SendCmd(CMD33, ed) == 0 
                && SD_SendCmd(CMD38, 0) == 0 
                && __SD_WaitReady())    /* Erase sector block */
    			res = RES_OK;	/* FatFs does not check result of this command */
            break;
        case MMC_GET_TYPE:      /* Get card type */
            *buff = SD_Type;
            res = RES_OK;
            break;
        case MMC_GET_CSD:       /* Get CSD */
    		if (!SD_GetCSD(buff))
                res = RES_OK;
            break;
        case MMC_GET_CID:       /* Get CID */
            if(!SD_GetCID(buff))
                res = RES_OK;
            break;
        case MMC_GET_OCR:       /* Get OCR */
            break;
        case MMC_GET_SDSTAT:    /* Get SD status */
            *buff = SD_status();
            res = RES_OK;
            break;
        case ISDIO_READ:        /* Read data form SD iSDIO register */
        case ISDIO_WRITE:       /* Write data to SD iSDIO register */
        case ISDIO_MRITE:       /* Masked write data to SD iSDIO register */
        default:
            res = RES_PARERR;
            break;
    }

    
    return res;
}
/*------------------------------------------------------------------------------------------------------------------------
--  �� �� ��:   EXTI15_10_IRQHandler
--  ����������  EXTI13 �ⲿ�жϴ������
--  ��ڲ�����  ��
--  �� �� ֵ��  ��
--  ˵    ����  ������������ߵ����������ж�
--------------------------------------------------------------------------------------------------------------------------
--  ��    �ߣ�  ������                                          ��    �ڣ�  2013��05��30��
-------------------------------------------------------------------------------------------------------------------------*/
void EXTI15_10_IRQHandler(void)
{

    if(EXTI_GetITStatus(EXTI_Line13) == SET){


        // ����һ���źţ� ����һ����ʱ�� ����ѭ������
        EXTI_ClearITPendingBit(EXTI_Line13);                        //����ж������־
    }

/*
    if( SD_IsPlugIn() ){                                        // �ɵ���==>����
        SD_PWR_EN();
    }else{                                                      // �ɲ���==>����
        SD_PWR_DIS();        
    }
    */
}
