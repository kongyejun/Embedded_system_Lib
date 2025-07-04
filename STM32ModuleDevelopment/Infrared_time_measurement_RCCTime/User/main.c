#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "TIMER_COUNTER.h"
#include "IR_thru_beam_counter.h"

uint16_t T_10ms=0,T_s=0,T_m=0,T_h=0;
int main(void){
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//------------设置分组
	//为了确保同一的分组方式，在main开头确定分组方式
	COUNTER_TIME_Init();
	OLED_Init();
	CountSensor_init();
	OLED_ShowString(1,1,"Time:");
	while(1){
	OLED_ShowNum(2,2,T_h,2);OLED_ShowChar(2,4,'.');
	OLED_ShowNum(2,5,T_m,2);OLED_ShowChar(2,7,'.');
	OLED_ShowNum(2,8,T_s,2);OLED_ShowChar(2,10,'.');
	OLED_ShowNum(2,11,T_10ms,2);
	}
	return 0;
}
/*
description：计时规则
*/
void TIM2_IRQHandler(void)//当前设置的定时时长为1s
{

	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	if(T_10ms==100)
	{
		T_s+1>59?(T_s=0,(T_m+1>60?(T_m=0,T_h++):++T_m)):++T_s;
		if(T_h==24)T_h=0;
		T_10ms=0;
	}
	T_10ms++;
}
/*
@description：采用双边沿触发方式，在物体遮挡时开启计时，离开时停止
*/
void EXTI9_5_IRQHandler(void)
{
	static uint16_t count_sensor;
	static FunctionalState T_State=ENABLE;
	
	if(EXTI_GetITStatus(EXTI_Line8)==SET){//判断是否为8号信号通道引起的中断
		EXTI_ClearITPendingBit(EXTI_Line8);//清除中断标志位
		count_sensor++;
		TIM_Cmd(TIM2,T_State);
		T_State=!T_State;
	}
}