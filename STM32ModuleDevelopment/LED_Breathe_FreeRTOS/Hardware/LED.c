#include "LED.h"
#include "Delay.h"
#include "ASSERT.h"

static GPIO_TypeDef* LED_GPIOx;

int LED_Init(GPIO_TypeDef* GPIOx,uint16_t Pin){
	if(GPIOx == GPIOA){
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	}else if(GPIOx == GPIOB){
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	}else if(GPIOx == GPIOC){
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	}else{
		return 0;
	}
	
	GPIO_InitTypeDef GPIO_struct;
	GPIO_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_struct.GPIO_Pin = Pin;
	GPIO_struct.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOx,&GPIO_struct);
	LED_GPIOx = GPIOx;
	return 1;
}


void LED_SetPin(uint16_t Pin ,uint8_t value){
	if(value == Bit_SET)
		GPIO_WriteBit(LED_GPIOx,GPIO_Pin_0 << Pin,Bit_SET);
	else
		GPIO_WriteBit(LED_GPIOx,GPIO_Pin_0 << Pin,Bit_RESET);
}

void LED_Running_Water_lamps(uint16_t Start_Pin,uint16_t End_Pin){
	int i;
	for(i=Start_Pin;i<=End_Pin;i++){
		LED_SetPin(i,Bit_SET);
		Delay_ms(500);
		LED_SetPin(i,Bit_RESET);
	}
}

void LED_Running_Water_lamps_FOR_RTOS(uint16_t Start_Pin,uint16_t End_Pin,uint8_t* Flag){
	if(*Flag & 0x10){
		LED_SetPin(Start_Pin + (*Flag & 0x0F),Bit_SET);
		*Flag = *Flag & 0x0F;//消除设置高电平标志位
	}else{
		LED_SetPin(Start_Pin + (*Flag),Bit_RESET);
		*Flag = (*Flag+1)<=(End_Pin)?(*Flag+1):Start_Pin;
		*Flag = (*Flag&0x07) | 0x10;
	}
}
