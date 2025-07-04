#include "LED.h"

void OFF_LED(void){
    gpio_set_pin(RED_LED_GPIONUM,GPIO_PV_HIGH);
    gpio_set_pin(GREEN_LED_GPIONUM,GPIO_PV_HIGH);
}

void LED_Init(void){
    gpio_init();//启动GPIO时钟
    gpio_set_drive_mode(RED_LED_GPIONUM,GPIO_DM_OUTPUT);
    gpio_set_drive_mode(GREEN_LED_GPIONUM,GPIO_DM_OUTPUT);
}