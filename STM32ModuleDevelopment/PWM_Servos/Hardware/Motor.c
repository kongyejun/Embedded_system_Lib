#include "Motor.h"

void Motor_Init(void)
{
	PWM_Init();
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIOA_init;
	GPIOA_init.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIOA_init.GPIO_Pin=GPIO_Pin_4|GPIO_Pin_5;
	GPIOA_init.GPIO_Speed=GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&GPIOA_init);

}

void MOTOR_set_Speed(int16_t speed)
{
	if(speed>=0){
		GPIO_SetBits(GPIOA,GPIO_Pin_4);
		GPIO_ResetBits(GPIOA,GPIO_Pin_5);
		PWM_SetCompare3(speed);
	}
	else{
		GPIO_SetBits(GPIOA,GPIO_Pin_5);
		GPIO_ResetBits(GPIOA,GPIO_Pin_4);
		PWM_SetCompare3(speed=-speed);
	}
}