#include "OLED_I2CPeriph.h"
#include "KEY.h"
#include "Delay.h"
#include "MOTOR_BASE.h"
#include "stm32f10x_gpio.h"
#include "LED.h"
#include "RTOS_App.h"

int main(void){
	OLED_Init();
    LED_Init();
    KEY_Init();
    MOTOR_Init();
    SystemStart_Task();
    while(1){ 
        ;
    }
}
