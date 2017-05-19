#include "hal_spi.h"

void SPI1_Init(uint16_t baud)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO | RCC_APB2Periph_SPI1,ENABLE);
	//--- ʹ��SPI1ʱ��
	
	//--- ����SPI1��SCK,MISO MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			  // ���ù���
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//--- SPI1����
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

   	//--- ʹ��SPI2��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB ,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	//--- ����SPI2��SCK,MISO MOSI 
	GPIO_InitStructure.GPIO_Pin 	=  	GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed 	= 	GPIO_Speed_10MHz;  	
	GPIO_InitStructure.GPIO_Mode 	= 	GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//--- SPI2���á�������������������������������������������������������������������������������������
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;		//����		:˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;							//ģʽ		:����ģʽ
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;						//���ݴ�С	:8λģʽ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;								//��̬		:ʱ������Ϊ��
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;							//λ����	:λ�ڵ�һ��������
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;								//NSS����	:�ڲ�����
	SPI_InitStructure.SPI_BaudRatePrescaler = baud; 						//�ٶ�		:256����Ƶ��Ϊ�����ģ��
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;						//λ����	:�Ӹ��ֽڿ�ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 7;								//У��		:�˴��������
	SPI_Init(SPI2, &SPI_InitStructure);				    

	//--- SPI1ʹ�ܡ�����������������������������������������������������������������������������������
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

