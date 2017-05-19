#include "hal_spi.h"

void SPI1_Init(uint16_t baud)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO | RCC_APB2Periph_SPI1,ENABLE);
	//--- 使能SPI1时钟
	
	//--- 配置SPI1的SCK,MISO MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			  // 复用功能
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//--- SPI1配置
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = baud;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);	
		
	SPI_Cmd(SPI1, ENABLE); 
}

uint8_t SPI1_ReadWrite(uint8_t writedat)
{
	/* Loop while DR register in not emplty */
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE) == RESET);
	
	/* Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(SPI1, writedat);
	
	/* Wait to receive a byte */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	
	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SPI1);
}


void SPI2_Init(uint16_t baud)
{
  	SPI_InitTypeDef SPI_InitStructure;
   	GPIO_InitTypeDef GPIO_InitStructure;

   	//--- 使能SPI2的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB ,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	//--- 配置SPI2的SCK,MISO MOSI 
	GPIO_InitStructure.GPIO_Pin 	=  	GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed 	= 	GPIO_Speed_10MHz;  	
	GPIO_InitStructure.GPIO_Mode 	= 	GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//--- SPI2配置。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;		//方向		:双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;							//模式		:主机模式
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;						//数据大小	:8位模式
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;								//稳态		:时钟悬空为低
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;							//位捕获	:位于第一个上升沿
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;								//NSS管理	:内部管理
	SPI_InitStructure.SPI_BaudRatePrescaler = baud; 						//速度		:256倍分频，为最低速模拟
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;						//位传输	:从高字节开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;								//校验		:此处不大理解
	SPI_Init(SPI2, &SPI_InitStructure);				    

	//--- SPI1使能。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。
	SPI_Cmd(SPI2, ENABLE);
}
uint8_t SPI2_ReadWrite(uint8_t writedat)
{					 
   /* Loop while DR register in not emplty */
   while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

   /* Send byte through the SPI1 peripheral */
   SPI_I2S_SendData(SPI2, writedat);

   /* Wait to receive a byte */
   while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);

   /* Return the byte read from the SPI bus */
   return SPI_I2S_ReceiveData(SPI2);
}	

