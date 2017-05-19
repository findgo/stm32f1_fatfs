#ifndef _USER_DIRVER_UART_H_
#define _USER_DIRVER_UART_H_

#include "app_cfg.h"


#define USART_USING1   					USART1
#define USART_USING1_IRQ				USART1_IRQn

#define USART_USING1_TX_PORT			GPIOA
#define USART_USING1_TX_PIN				GPIO_Pin_9

#define USART_USING1_RX_PORT			GPIOA
#define USART_USING1_RX_PIN				GPIO_Pin_10

#define USART_USING1_GPIO_PeriphClock_EN()  do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE); }while(0)
#define USART_USING1_PeriphClock_EN()	do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);}while(0)


void Usart_Configuration(void);

#endif

