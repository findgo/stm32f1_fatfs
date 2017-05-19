

#ifndef __HAL_SPI_H_
#define __HAL_SPI_H_

#include "app_cfg.h"


/*  spi1 
    MISO -- PA6
    MOSI -- PA7
    SCK -- PA5
*/



void SPI1_Init(uint16_t baud);
uint8_t SPI1_ReadWrite(uint8_t dat);
void SPI2_Init(uint16_t baud);
uint8_t SPI2_ReadWrite(uint8_t dat);

#endif


