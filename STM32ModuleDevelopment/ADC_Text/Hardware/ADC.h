#ifndef _ADC_H_
#define _ADC_H_
#include "stm32f10x.h"
#include "Delay.h"
FunctionalState ADC_Four_Init();

//void ADC_Start();//----------------------------------方案一
//void ADC_DMA_Start();//------------------------------方案二
void ADC_Filtering_Strat();//--------------------------方案三
#endif