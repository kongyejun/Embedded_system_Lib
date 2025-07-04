#include "MOTOR_BASE.h"

#define MOTOR_AIN1 GPIO_Pin_4
#define MOTOR_AIN2 GPIO_Pin_5
#define MOTOR_PWM  GPIO_Pin_6

void PWM_SetCompare1(uint16_t Compare){
	TIM_SetCompare1(TIM3, Compare);
}

void Motor_SetSpeed(int8_t Speed){
	if (Speed > 0){
		GPIO_SetBits(GPIOA, MOTOR_AIN1);
		GPIO_ResetBits(GPIOA, MOTOR_AIN2);
		PWM_SetCompare1(Speed);
	}else if(Speed < 0){
		GPIO_ResetBits(GPIOA, MOTOR_AIN1);
		GPIO_SetBits(GPIOA, MOTOR_AIN2);
		PWM_SetCompare1(-Speed);
	}else{
		GPIO_ResetBits(GPIOA, MOTOR_AIN1);
		GPIO_ResetBits(GPIOA, MOTOR_AIN2);
		PWM_SetCompare1(0);
    }
}

void PWM_Init(void){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	TIM_InternalClockConfig(TIM3);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;		 //ARR
	TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1;   //PSC
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
	
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;//CCR
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
	TIM_Cmd(TIM3, ENABLE);
}

void MOTOR_Init(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    GPIO_InitTypeDef GPIO_InitStruct; //定义结构体变量
    GPIO_InitStruct.GPIO_Pin = MOTOR_AIN1 | MOTOR_AIN2;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = MOTOR_PWM;
    GPIO_Init(GPIOA,&GPIO_InitStruct);
    PWM_Init();
}
