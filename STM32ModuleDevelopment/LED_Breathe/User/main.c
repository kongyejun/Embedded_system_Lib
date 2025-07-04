#include "stm32f10x.h"
#include "Delay.h"
int main(){
	uint16_t i;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All&~0xE000;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_Write(GPIOB,0xFFFF);
	GPIO_Write(GPIOA,0xFFFF);
	while(1){
		GPIO_Write(GPIOA,0x0000);
		GPIO_Write(GPIOB,0x0000);
		// for(i=0;i<13;i++){
		// 	GPIO_Write(GPIOA, ~((uint16_t)0x01<<i));
		// 	Delay_ms(500);
		// }
		// GPIO_Write(GPIOA,~GPIO_Pin_15);
		// Delay_ms(500);
		// GPIO_Write(GPIOA,0xFFFF);

		// GPIO_Write(GPIOB, ~GPIO_Pin_0);
		// Delay_ms(500);
		// GPIO_Write(GPIOB, ~GPIO_Pin_1);
		// Delay_ms(500);
		// for(i=5;i<15;i++){
		// 	GPIO_Write(GPIOB, ~((uint16_t)0x01<<i));
		// 	Delay_ms(500);
		// }

		// GPIO_Write(GPIOB,0xFFFF);
	}
	
}