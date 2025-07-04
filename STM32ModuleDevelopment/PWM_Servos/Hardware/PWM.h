#ifndef _PWM_H_
#define _PWM_H_
#include "stm32f10x.h"

void PWM_Init();
void PWM_SetCompare3(uint16_t data);
#endif