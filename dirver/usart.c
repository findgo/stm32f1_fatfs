/**
  ******************************************************************************
  * @file    usart.c
  * @author  
  * @version 
  * @date    
  * @brief   
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
#include "usart.h"


/**
  * @brief	usart≈‰÷√
  * @param	None
  * @note	
  * @retval  None
  */

void Usart_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	USART_ClockInitTypeDef USART_ClockInitStruct;
	USART_InitTypeDef USART_InitStruct;

	USART_USING1_GPIO_PeriphClock_EN();
	USART_USING1_PeriphClock_EN();

	//PB6 ?????USAR1 TX, PB7 ?????USART1 RX
	GPIO_InitStruct.GPIO_Pin   = USART_USING1_TX_PIN;//TX AF mode
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(USART_USING1_TX_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Pin   = USART_USING1_RX_PIN;//RX AF mode
	GPIO_Init(USART_USING1_RX_PORT, &GPIO_InitStruct);


	NVIC_InitStruct.NVIC_IRQChannel = USART_USING1_IRQ;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority  =  1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	USART_ClockStructInit(&USART_ClockInitStruct);
	USART_ClockInit(USART_USING1, &USART_ClockInitStruct);

	USART_InitStruct.USART_BaudRate = 115200;
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStruct.USART_HardwareFlowControl = DISABLE;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;// 8 bit
	USART_Init(USART_USING1, &USART_InitStruct);

	USART_ClearFlag(USART_USING1, USART_FLAG_TXE | USART_IT_RXNE | USART_FLAG_TC);
	USART_ITConfig(USART_USING1, USART_IT_TXE,	DISABLE);
	USART_ITConfig(USART_USING1, USART_IT_TC, DISABLE);
	USART_ITConfig(USART_USING1, USART_IT_RXNE, ENABLE);

	USART_Cmd(USART_USING1, ENABLE);//enable USART1
}

