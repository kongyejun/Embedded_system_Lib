#ifndef __LED_H__
#include "stm32f10x.h"
#define LED_Pin GPIO_Pin_10

void LED_Init(void);
void Lift_LEDState(uint8_t state);
#endif
