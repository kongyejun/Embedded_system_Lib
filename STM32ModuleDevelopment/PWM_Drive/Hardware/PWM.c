#include "PWM.h"

void PWM_Init()
{
	//开启时钟信号
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	//配置OC1端口所需要的GPIO口
	GPIO_InitTypeDef GPIOA_Pin0;
	GPIOA_Pin0.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIOA_Pin0.GPIO_Pin=GPIO_Pin_0;
	GPIOA_Pin0.GPIO_Speed=GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&GPIOA_Pin0);
	
	//配置时基单元
		//设置时基单元的时钟源为外部属性
	TIM_InternalClockConfig(TIM2);
		//创建时基单元初始化结构体
	TIM_TimeBaseInitTypeDef TIM2_INST;
	TIM2_INST.TIM_ClockDivision=TIM_CKD_DIV1;//滤波器采样频率
	TIM2_INST.TIM_CounterMode=TIM_CounterMode_Up;
	
	TIM2_INST.TIM_Period=100-1;
	TIM2_INST.TIM_Prescaler=720-1;//使PWM波的频率为1KHz
	
	TIM2_INST.TIM_RepetitionCounter=0;
		//调用初始化函数
	TIM_TimeBaseInit(TIM2,&TIM2_INST);
	//配置OC1端口
	TIM_OCInitTypeDef TIM2_OCInitSturct;
	TIM_OCStructInit(&TIM2_OCInitSturct);
	TIM2_OCInitSturct.TIM_OCMode=TIM_OCMode_PWM1;//小于CCR时是有效电平，大于等于时无效电平
	TIM2_OCInitSturct.TIM_Pulse=0;
	TIM2_OCInitSturct.TIM_OCPolarity=TIM_OCPolarity_Low;
	TIM2_OCInitSturct.TIM_OutputState=ENABLE;
	
	TIM_OC1Init(TIM2,&TIM2_OCInitSturct);
	
	//开启时基单元
	TIM_Cmd(TIM2,ENABLE);
}

void PWM_Hight_Set(uint16_t data){
	TIM_SetCompare1(TIM2,data);
}