#include "ASSERT.h"
#include "stm32f10x.h"
#include "Delay.h"
void ASSERT_Init(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIO_Struct;
	GPIO_Struct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Struct.GPIO_Pin = GPIO_Pin_8;
	GPIO_Struct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_Struct);
}

void ASSERT_Hapend(){
	while(1){
		GPIO_WriteBit(GPIOA,GPIO_Pin_8,Bit_SET);
		Delay_ms(500);
		GPIO_WriteBit(GPIOA,GPIO_Pin_8,Bit_RESET);
		Delay_ms(500);
	}
}
