#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "LED.h"
#include "ASSERT.h"
#include "RTOS_APP.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
int main(void){
	int err;
	ASSERT_Init();
	err = LED_Init(GPIOA,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2);
	if(!err){ASSERT_Hapend();}
	FreeRTOS_Init();
	while(1);
}
