#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "IR_thru_beam_counter.h"

void CountSensor_IN()
{
	//配置EXTI寄存器
	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line=EXTI_Line10;
	EXTI_InitStruct.EXTI_LineCmd=ENABLE;
	EXTI_InitStruct.EXTI_Mode=EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger=EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStruct);
	//配置NVIC寄存器
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel=EXTI15_10_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=2;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=1;
	NVIC_Init(&NVIC_InitStruct);
}

int main(void){
	
	CountSensor_IN();
	OLED_Init();
	CountSensor_init();
	OLED_ShowString(1,1,"COUNT:");
	while(1)
	{
		 ;
	}
	
	return 0;
} 

void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line10)==SET){
		EXTI_ClearITPendingBit(EXTI_Line10);
		OLED_ShowNum(1,7,GetCountSensor(),5);
	}
}