#ifndef __I2C_BASE_H__
#include "stm32f10x.h"

void I2C1_Init(void);
void I2C1_Write(uint8_t *pBuffer,uint8_t SlaveADDR ,uint8_t WriteAddr ,uint16_t Len);
void I2C1_Fill(uint8_t *pBuffer,uint8_t SlaveADDR ,uint8_t WriteAddr ,uint8_t Len);
void I2C1_Read(uint8_t *pBuffer,uint8_t SlaveADDR ,uint8_t ReadAddr,uint8_t Len);
#endif
