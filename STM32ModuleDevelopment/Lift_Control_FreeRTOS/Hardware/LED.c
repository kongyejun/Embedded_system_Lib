#include "LED.h"
#include "stm32f10x_gpio.h"

void Lift_LEDState(uint8_t state){
    if(state){// 电梯工作 绿灯亮 输出高电平
        GPIO_SetBits(GPIOA,LED_Pin);
    }else{
        GPIO_ResetBits(GPIOA,LED_Pin);
    }
}

void LED_Init(){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = LED_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    Lift_LEDState(0);// 电梯暂停  红灯亮 默认
}
