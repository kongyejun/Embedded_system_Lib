#include "stm32f10x.h"                  // Device header
#include "Motor.h"
#include "OLED.h"
#include "Key.h"

int main(void){
	uint16_t speed=0;
	Motor_Init();
	OLED_Init();
	Key_Init(GPIO_Pin_7);
	OLED_ShowString(1,1,"Speed:");
	OLED_ShowNum(1,7,speed,3);
	
	while(1){
	if(Key_value(GPIO_Pin_7)==RESET){
		speed+=10;		
		if(speed>100)speed=0;
		MOTOR_set_Speed(speed);
		OLED_ShowNum(1,7,speed,3);
		}
	}
	return 0;
}
