#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "TIMER_COUNTER.h"

uint16_t T_AD_Count=0,T_Count=0;
int main(void){
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//------------设置分组
	//为了确保同一的分组方式，在main开头确定分组方式
	COUNTER_TIME_Init();
	OLED_Init();
	OLED_ShowString(1,1,"COUNT:");
	while(1){
		OLED_ShowNum(1,7,T_AD_Count,2);
		OLED_ShowNum(1,10,COUNT_TIME_Data(),2);
	}
	return 0;
}
/*
description：计时规则
*/
void TIM2_IRQHandler(void)
{
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	T_AD_Count++;
	T_Count=0;
}
