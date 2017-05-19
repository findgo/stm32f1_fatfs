

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


/*** 本地变量定义*******************************************************************************/
static uint8_t SD_Type = 0;//SD卡的类型
static DSTATUS SD_Stat = STA_NOINIT;
/*
 * 成功: 0
 * 失败: other
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


//释放SPI总线
static void __SD_DisSelect(void)
{
    SD_CSH();
    SD_SPI_ReadWrite(0xff);//提供额外的8个时钟
}
//选择sd卡,并且等待卡准备OK
//返回值:0,成功;1,失败;
static uint8_t __SD_Select(void)
{
    SD_CSL();
    if(__SD_WaitReady() == 0)
        return 0;//等待成功
    __SD_DisSelect();
    
    return 1;//等待失败
}
/*
 * 检测 sd卡是否插入
 * 1:  插入
 * 0:  弹出
*/
uint8_t SD_DetectPugIn(void)
{
    return SD_IsPlugIn();
}
/*
 * 检测 sd卡是否写保护
 * 1:  保护
 * 0:  未保护
*/
static uint8_t SD_DetectWriteProtectEnabled(void)
{
    return SD_IsWriteEnable();
}
/********************************************************************************************************
** 函数: SD_SendCmd, 等待SD卡回应
**------------------------------------------------------------------------------------------------------
** 参数: cmd    :   命令
**       arg    :   命令参数
** 返回: SD卡返回的响应 
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
** 函数: SD_PortInit, SD卡硬件接口初始化
**------------------------------------------------------------------------------------------------------
** 参数: 无
** 返回: 无
** NOTE: 
********************************************************************************************************/
void SD_PortInit( void )
{   
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    // 配置IO 
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC,ENABLE);          // 打开对应IO的外设时钟
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_4;        // PA0--PWR_EN PA4--CS
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;                    // 卡输入检测,正常为低、弹出为高
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    PWR_BackupAccessCmd(ENABLE);                                    // 允许修改RTC
    RCC_LSEConfig(RCC_LSE_OFF);                                     // 关闭外部低速时钟信号功能,这样才能用PC13~PC15     

    // 配置卡插入检测中断
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;                // 外部中断2
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;       //抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2 ;                    //子优先级  
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                     // 使能
    NVIC_Init(&NVIC_InitStructure);
    
    //用于配置AFIO外部中断配置寄存器AFIO_EXTICR1，用于选择EXTI13外部中断的输入源是PC1。
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource13);        // 外部中断配置AFIO--ETXI1
    EXTI_InitStructure.EXTI_Line    = EXTI_Line13;                      // PC1 作为键盘K3 检测状态
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;              // 中断模式
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;      // 双边沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure); 

    // SD及其他初始化
    SD_SPI_InitLowBaud();   
    SD_CSH();
    SD_PWR_EN();
}


/********************************************************************************************************
** 函数: SD_Init, 初始化SD卡
**------------------------------------------------------------------------------------------------------
** 参数: 无
** 返回: 0      :   初始化成功
**       其他   :   参见SD卡返回值
** NOTE: 
********************************************************************************************************/
DSTATUS SD_Init(void)
{ 
    uint8_t r1,i;      
    uint8_t buf[4]; 
    uint16_t retry;
 
    SD_Type = 0;//默认无卡

    // 初始化SD卡到SPI模式 写保护口 插入检测
    SD_SPI_InitLowBaud();   

    if(!SD_DetectPugIn())
        return STA_NODISK;

    // 卡识别模式 默认识别时钟频率 < 400KHZ
    SD_CSH();
    for(i = 0;i < 12; i++)
        SD_SPI_ReadWrite(0xff);//发送最少74个高脉冲
    retry = 200;
    do{
        r1 = SD_SendCmd(CMD0,0);// send COM0 go to idle
    }while((r1 != 0x01) && --retry);
    // 进入spi模式失败
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
            do {//等待退出IDLE模式                
                r1 = SD_SendCmd(ACMD41,0);//发送CMD41
            }while(r1 && --retry);
        }else{
            SD_Type = CT_MMC;//MMC V3
            retry = 400;
            do {//等待退出IDLE模式
                r1 = SD_SendCmd(CMD1,0);//发送CMD1
            }while(r1 && --retry);  
        }
        
       retry = 100;
       do {//等待退出IDLE模式
           r1 = SD_SendCmd(CMD16, 512);//设置sector size 为512
       }while(r1 && --retry);
       if(retry == 0)
           SD_Type = 0;
    }else if(r1 == 0x01){ //SD V2.0
        for(i = 0; i < 4; i++)
            buf[i] = SD_SPI_ReadWrite(0xff);  //Get trailing return value of R7 resp
        
        if(!(buf[2] == 0x01 && buf[3] == 0xAA)) {//卡是否支持2.7~3.6V
            r1 = 1;
            goto EXIT_FUNC;
        }
        
        retry = 400;
        do{
            r1 = SD_SendCmd(ACMD41,0x40000000);//发送CMD41
        }while(r1 && --retry);

        //鉴别SD2.0卡版本开始
        if(retry && SD_SendCmd(CMD58,0) == 0){ /* Check CCS bit in the OCR */
            for(i = 0; i < 4; i++)
                buf[i] = SD_SPI_ReadWrite(0xFF);//得到OCR值
            if(buf[0] & 0x40)
                SD_Type = CT_SD2 | CT_BLOCK;    //检查CCS
            else 
                SD_Type = CT_SD2;   
        }
    }
	
EXIT_FUNC:
    __SD_DisSelect();
    SD_SPI_InitHighBaud();//高速
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
        SD_PWR_DIS();//如果初始化失败，关闭卡的电源

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
** 函数: __SD_ReceiveData, 从SD卡中读回指定长度的数据，放置在给定位置
**------------------------------------------------------------------------------------------------------
** 参数: data   :   获取数据的存放处
**       len    :   数据长度
** 返回: 0      :   成功
**       其他   :   错误 参见SD卡返回值
** NOTE: 
********************************************************************************************************/
static uint8_t __SD_ReceiveData(uint8_t *data, uint16_t len)
{ 
    uint16_t retry = 510;//等待次数
    
    while ((SD_SPI_ReadWrite(0xFF) != 0xFE) && --retry);//等待得到准确的回应  
  
    if (retry == 0)
        return 1;//得到回应失败   

    while(len--)//开始接收数据
    {
        *data = SD_SPI_ReadWrite(0xFF);
        data++;
    }
    //下面是2个伪CRC（dummy CRC）
    SD_SPI_ReadWrite(0xFF);
    SD_SPI_ReadWrite(0xFF);  
    
    return 0;//读取成功
}           

//获得SD卡类型
uint8_t SD_GetSDType(void)
{
    return SD_Type;
}
/********************************************************************************************************
** 函数: SD_GetCID, 获取SD卡的CID信息，包括制造商信息
**------------------------------------------------------------------------------------------------------
** 参数: cid_data   :   (存放CID的内存，16Byte输出）
** 返回: 0          :   初始化成功
**       其他       :   错误 参见SD卡返回值
** NOTE: 详细结构参见==http://blog.csdn.net/lwj103862095/article/details/38335709
********************************************************************************************************/
uint8_t SD_GetCID( uint8_t *cid_data )
{
    uint8_t r1;   
    
    //发CMD10命令，读CID
    if(SD_SendCmd(CMD10,0) == 0x00){
        r1 = __SD_ReceiveData(cid_data,16);//接收16个字节的数据  
    }
    __SD_DisSelect();//取消片选
    
    if(r1)
        return 1;

    return 0;
}           

/********************************************************************************************************
** 函数: SD_GetCID, 获取SD卡的CSD信息，包括容量和速度信息
**------------------------------------------------------------------------------------------------------
** 参数: csd_data   :   (存放CSD的内存，16Byte输出）
** 返回: 0          :   初始化成功
**       其他       :   错误 参见SD卡返回值
** NOTE: 详细结构参见==http://blog.csdn.net/lwj103862095/article/details/38335709
********************************************************************************************************/
uint8_t SD_GetCSD(uint8_t *csd_data)
{
    uint8_t r1;
    
    if(SD_SendCmd(CMD9,0) == 0){    //发CMD9命令，读CSD
        r1 = __SD_ReceiveData(csd_data, 16);//接收16个字节的数据 
    }
    __SD_DisSelect();//取消片选
    
    if(r1)
        return 1;
    else 
        return 0;
}  

/********************************************************************************************************
** 函数: SD_GetByteCap, 获取SD卡以扇区为单位的容量大小   
**------------------------------------------------------------------------------------------------------
** 返回:  :   SD容量
** NOTE: 
********************************************************************************************************/
uint32_t SD_GetSectorCount(void)
{
    uint8_t n;
    uint8_t csd[16];
    uint16_t csize;
    uint32_t Capacity;
    
    //取CSD信息，如果期间出错，返回0
    if(SD_GetCSD(csd) != 0) 
        return 0; 
    
    //如果为SDHC卡，按照下面方式计算
    if((csd[0] & 0xC0) == 0x40){  /* SDC ver 2.00 */
		csize = csd[9] + ((uint16_t)csd[8] << 8) + ((uint16_t)(csd[7] & 63) << 16) + 1;
        Capacity = (uint32_t)csize << 10;//得到扇区数               
    }else{      /* SDC ver 1.XX or MMC ver 3 */
        n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
        csize = (csd[8] >> 6) + ((uint16_t)csd[7] << 2) + ((uint16_t)(csd[6] & 3) << 10) + 1;
        Capacity= (uint32_t)csize << (n - 9);//得到扇区数   
    }
    return Capacity;
}

/********************************************************************************************************
** 函数: SD_ReadMultiBlock, 读SD卡的多个block(实际测试过)   
**------------------------------------------------------------------------------------------------------
** 参数: sector :   取地址（sector值，非物理地址） 
**       buffer :   数据存储地址（大小至少512byte） 
**       blockcnt  :   连续读count个block  
** 返回: 0      :   成功
**       其他   :   失败
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
        sector <<= 9;//转换为字节地址

    cmd = (blockcnt == 1) ? CMD17 : CMD18; 
    
    if(SD_SendCmd(cmd,sector) != 0x00){
        __SD_DisSelect();//取消片选
        return RES_ERROR;
     }
    
    do{
        if(__SD_ReceiveData(buf,512) != 0x00) 
            break;      // 接收数据
        buf += 512;
        --blockcnt;
    }while(blockcnt);
    
    if(cmd == CMD18){
        (void)SD_SendCmd(CMD12,0);   //多块读 发送停止命令
    }
    __SD_DisSelect();//取消片选
   
	return blockcnt ? RES_ERROR : RES_OK;	/* Return result */
}                                             
//向sd卡写入一个数据包的内容 512字节
//buf:数据缓存区
//cmd:指令
//返回值:0,成功; >0 失败;  
static uint8_t __SD_SendBlock(const uint8_t *buf,uint8_t cmd)
{
    uint16_t i;
        
    // 发送若干时钟后开始写
    SD_SPI_ReadWrite(0xff);
    SD_SPI_ReadWrite(0xff);
    SD_SPI_ReadWrite(0xff);

    SD_SPI_ReadWrite(cmd);  
    for(i = 0;i < 512; i++)
        SD_SPI_ReadWrite(buf[i]);
    // 发送两个 dummy CRC
    SD_SPI_ReadWrite(0xFF);//忽略crc
    SD_SPI_ReadWrite(0xFF);
    
    return __SD_WaitReady();
}
/********************************************************************************************************
** 函数: SD_WriteMultiBlock,写入SD卡的N个block(未实际测试过)                                
**------------------------------------------------------------------------------------------------------
** 参数: sector :   地址（sector值，非物理地址） 
**       data   :   写入的512字节* blockcnt
**       blockcnt    :   写入的block数目
** 返回: 0      :   成功
**       其他   :   错误 参见SD卡返回值
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
        sector <<= 9;//转换为字节地址

    if(blockcnt == 1)
        cmd = CMD24;
    else{
        cmd = CMD25;
        /*
        // 如果目标卡不是MMC卡,启动ACMD23指令使能预擦除
        if(SD_Type != SD_TYPE_MMC){
            SD_SendCmd(CMD55,0,0x01);   
            SD_SendCmd(ACMD23,cnt,0X01);//发送指令  
         }  
        */
    }
    if(SD_SendCmd(cmd,sector) != 0x00){
        __SD_DisSelect();
        return RES_ERROR;
     }
    
    if(blockcnt == 1){ // 写单扇区
        if(__SD_SendBlock(buf,0xFE) == 0)//写512个字节 
            blockcnt = 0;
    }else{ // 写多扇区
        do{
            if(__SD_SendBlock(buf,0xFC))//发送512个字节  
                break;
            buf += 512;
            --blockcnt;
        }while(blockcnt);
        (void)SD_SPI_ReadWrite(0xFD);//发送结束令牌
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
--  函 数 名:   EXTI15_10_IRQHandler
--  功能描述：  EXTI13 外部中断处理程序
--  入口参数：  无
--  返 回 值：  无
--  说    明：  当卡被插入或者弹出触发该中断
--------------------------------------------------------------------------------------------------------------------------
--  作    者：  胡福成                                          日    期：  2013年05月30日
-------------------------------------------------------------------------------------------------------------------------*/
void EXTI15_10_IRQHandler(void)
{

    if(EXTI_GetITStatus(EXTI_Line13) == SET){


        // 发送一个信号， 启动一个延时。 由主循环处理
        EXTI_ClearITPendingBit(EXTI_Line13);                        //清除中断请求标志
    }

/*
    if( SD_IsPlugIn() ){                                        // 由弹出==>插入
        SD_PWR_EN();
    }else{                                                      // 由插入==>弹出
        SD_PWR_DIS();        
    }
    */
}
