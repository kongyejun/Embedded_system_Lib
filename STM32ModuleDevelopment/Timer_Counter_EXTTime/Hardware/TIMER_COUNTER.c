#include "stm32f10x.h"

void COUNTER_TIME_Init()
{
	//开启时钟信号
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);

	//配置GPIO口
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_0;//-------------------------------引脚不能乱选，在引脚表中有规定！！！！！！！！！！！！！！！
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	//配置时基单元
		//设置时基单元的时钟源为外部属性
	TIM_ETRClockMode2Config(TIM2,TIM_ExtTRGPSC_OFF,TIM_ExtTRGPolarity_NonInverted,0x00);
		//创建时基单元初始化结构体
	TIM_TimeBaseInitTypeDef TIM2_INST;
	TIM2_INST.TIM_ClockDivision=TIM_CKD_DIV1;//滤波器采样频率
	TIM2_INST.TIM_CounterMode=TIM_CounterMode_Up;
	TIM2_INST.TIM_Period=16-1;
	TIM2_INST.TIM_Prescaler=0;
	TIM2_INST.TIM_RepetitionCounter=0;
		//调用初始化函数
	TIM_TimeBaseInit(TIM2,&TIM2_INST);
	
	//清除中断标志位
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);
	
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
	
	//开启时基单元
	TIM_Cmd(TIM2,ENABLE);
}

uint16_t COUNT_TIME_Data(void)
{
	return TIM_GetCounter(TIM2);
}

//void TIM2_IRQHandler(void)//当前设置的定时时长为1s
//{
//	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
//	Time_Num++;
//}
