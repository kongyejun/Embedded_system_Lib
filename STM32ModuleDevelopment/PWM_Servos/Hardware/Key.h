#ifndef _KEY_H_
#define _KEY_H_
#include "stm32f10x.h"
void Key_Init(uint16_t GPIO_Pin_X);
uint16_t Key_value(uint16_t GPIO_Pin_x);

#endif