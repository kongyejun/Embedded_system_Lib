#ifndef _SERVOS_H_
#define _SERVOS_H_
#include "stm32f10x.h"
#include "PWM.h"
void Motor_Init(void);
void MOTOR_set_Speed(int16_t speed);
#endif