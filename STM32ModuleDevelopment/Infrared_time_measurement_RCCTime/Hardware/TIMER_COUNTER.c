#include "stm32f10x.h"

void COUNTER_TIME_Init()
{
	//开启时钟信号
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	
	//配置时基单元
		//选择时基单元
	TIM_InternalClockConfig(TIM2);
		//创建时基单元初始化结构体
	TIM_TimeBaseInitTypeDef TIM2_INST;
	TIM2_INST.TIM_ClockDivision=TIM_CKD_DIV1;//滤波器采样频率
	TIM2_INST.TIM_CounterMode=TIM_CounterMode_Up;
	TIM2_INST.TIM_Period=100;
	TIM2_INST.TIM_Prescaler=7200-1;
	TIM2_INST.TIM_RepetitionCounter=0;
		//调用初始化函数
	TIM_TimeBaseInit(TIM2,&TIM2_INST);
	
	//清除中断标志位
	TIM_ClearFlag(TIM2,TIM_IT_Update);
	
	//配置中断控制器
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	//配置NVIC
		//配置NVIC分组
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
		//配置中断设置
	NVIC_InitTypeDef NVIC_INIT;
	NVIC_INIT.NVIC_IRQChannel=TIM2_IRQn;
	NVIC_INIT.NVIC_IRQChannelCmd=ENABLE;
	NVIC_INIT.NVIC_IRQChannelPreemptionPriority=2;
	NVIC_INIT.NVIC_IRQChannelSubPriority=2;
	NVIC_Init(&NVIC_INIT);
	
	//开启定时器
	//TIM_Cmd(TIM2,ENABLE);
}

//void TIM2_IRQHandler(void)//当前设置的定时时长为1s
//{
//	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
//	Time_Num++;
//}
