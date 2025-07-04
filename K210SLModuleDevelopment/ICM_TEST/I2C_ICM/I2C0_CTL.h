#ifndef _I2C0_CTL_H
#define _I2C0_CTL_H
#include <stdint.h>

#define ADDRESS_WIDTH   7
#define I2C_CLK_SPEED   100000

void I2C_HardWare_Init(uint16_t addr);
uint16_t I2C_Write_Byte(uint8_t addr, uint8_t reg, uint8_t data);
uint16_t I2C_Read(uint8_t addr, uint8_t reg, uint8_t* data_buf, uint16_t length);
uint16_t I2C_Write(uint8_t addr, uint8_t reg, uint8_t* data,uint8_t Length);

void I2C_Read_ADDR(void);
#endif  /* _I2C_CTL_H */