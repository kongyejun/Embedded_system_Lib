#ifndef _ADC_H_
#define _ADC_H_
#include "stm32f10x.h"

void MY_ADC_Init(void);
uint16_t ADC_GetData(void);
uint16_t ADC_GetThisData(uint8_t ADC_Channe);//需修改初始化函数
#endif
