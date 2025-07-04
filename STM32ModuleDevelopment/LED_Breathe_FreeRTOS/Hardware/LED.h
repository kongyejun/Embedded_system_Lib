#ifndef _LED_H_
#define _LED_H_
#include "stm32f10x.h"
int LED_Init(GPIO_TypeDef* GPIOx,uint16_t Pin);
void LED_Running_Water_lamps(uint16_t Start_Pin,uint16_t End_Pin);
void LED_Running_Water_lamps_FOR_RTOS(uint16_t Start_Pin,uint16_t End_Pin,uint8_t* Flag);
#endif
