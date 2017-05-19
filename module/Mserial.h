/**
  ******************************************************************************
  * @file   Mserial.h
  * @author  
  * @version 
  * @date    
  * @brief      FIFO非阻塞串口通信，用于数据流传输
  ******************************************************************************
  * @attention      V1.2 jgb        201704023
  ******************************************************************************
  */

#ifndef __MOD_SERIAL_H_
#define __MOD_SERIAL_H_

#include "app_cfg.h"
#include <string.h>

#include "usart.h"


// 宏 控制支持的串口数量
#define COM_USE_NUM     1




enum{
    COM0,
    COM1,
    COM2,
    COM3
};

//宏 用于控制非阻塞方式下，设置发送，接收最大缓冲区
#define COM0_RX_MAX_SIZE   255
#define COM0_TX_MAX_SIZE   255

#define COM1_RX_MAX_SIZE   255
#define COM1_TX_MAX_SIZE   255

#define COM2_RX_MAX_SIZE   255
#define COM2_TX_MAX_SIZE   255

#define COM3_RX_MAX_SIZE   255
#define COM3_TX_MAX_SIZE   255

#define COM0PutByte(dat)  	(USART_SendData(USART_USING1, dat))
#define COM0GetByte()     	((uint8_t)USART_ReceiveData(USART_USING1))
#define COM0TxIEEnable()    	do{ USART_ITConfig(USART_USING1, USART_IT_TXE, ENABLE); }while(0)
#define COM0TxIEDisable()   	do{ USART_ITConfig(USART_USING1, USART_IT_TXE, DISABLE); }while(0)

#define COM1PutByte(dat)  	
#define COM1GetByte()     	
#define COM1TxIEEnable()    	
#define COM1TxIEDisable()   	

#define COM2PutByte(dat)  	
#define COM2GetByte()     	
#define COM2TxIEEnable()    	
#define COM2TxIEDisable()   	

#define COM3PutByte(dat)  	
#define COM3GetByte()     	
#define COM3TxIEEnable()    	
#define COM3TxIEDisable()   	

uint16_t Serial_WriteByte(uint8_t COM,uint8_t dat);
uint16_t Serial_WriteBuf(uint8_t COM,uint8_t *buf,uint16_t len);
uint16_t Serial_Read(uint8_t COM,uint8_t *buf,uint16_t len);
uint16_t serialRxValidAvail(uint8_t COM);
#define Serial_WriteStr(COM,str) Serial_WriteBuf(COM,(uint8_t *)str,strlen(str))


void COM0_TXE_Isr_callback(void);
void COM0_TXC_Isr_callback(void);
void COM0_RX_Isr_callback(void);


#endif

