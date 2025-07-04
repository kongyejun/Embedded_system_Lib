#include "Key.h"

void Key_Init(uint16_t GPIO_Pin_X)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitSturt;
	GPIO_InitSturt.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitSturt.GPIO_Pin=GPIO_Pin_X;
	GPIO_InitSturt.GPIO_Speed=GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&GPIO_InitSturt);
}

uint16_t Key_value(uint16_t GPIO_Pin_x)
{
	if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_x)==RESET){
		while(!GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_x));
		return RESET;
	}else{
		return SET;
	}
}