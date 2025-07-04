#include "KEYPAD.h"

int KEYPADL_ISR(void* ctx){
    timer_set_enable(TIMER_DEVICE_0,TIMER_CHANNEL_0,1);
    return 0;
}
int KEYPADM_ISR(void* ctx){
    timer_set_enable(TIMER_DEVICE_0,TIMER_CHANNEL_1,1);
    return 0;
}
int KEYPADR_ISR(void* ctx){
    timer_set_enable(TIMER_DEVICE_0,TIMER_CHANNEL_2,1);
    return 0;
}

void KEYPAD_Init(void){
    //启用全局中断
    sysctl_enable_irq();
    //外部中断初始化
    plic_init();
    //定时器初始化
    Timer_Init(TIMER_DEVICE_0,TIMER_CHANNEL_0,20,0,1,Time0_channel0_ISR,NULL);
    Timer_Init(TIMER_DEVICE_0,TIMER_CHANNEL_1,20,0,1,Time0_channel1_ISR,NULL);
    Timer_Init(TIMER_DEVICE_0,TIMER_CHANNEL_2,20,0,1,Time0_channel2_ISR,NULL);
    //设置GPIOHS工作模式
    gpiohs_set_drive_mode(KEYPAD_L_GPIOHSNUM,GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_drive_mode(KEYPAD_M_GPIOHSNUM,GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_drive_mode(KEYPAD_R_GPIOHSNUM,GPIO_DM_INPUT_PULL_UP);
    //设置GPIOHS中断触发方式(选择双边沿电平触发)
    gpiohs_set_pin_edge(KEYPAD_L_GPIOHSNUM,GPIO_PE_BOTH);
    gpiohs_set_pin_edge(KEYPAD_M_GPIOHSNUM,GPIO_PE_BOTH);
    gpiohs_set_pin_edge(KEYPAD_R_GPIOHSNUM,GPIO_PE_BOTH);
    //设置GPIOHS中断函数
    gpiohs_irq_register(KEYPAD_L_GPIOHSNUM,1,KEYPADL_ISR,NULL);
    gpiohs_irq_register(KEYPAD_M_GPIOHSNUM,1,KEYPADM_ISR,NULL);
    gpiohs_irq_register(KEYPAD_R_GPIOHSNUM,1,KEYPADR_ISR,NULL);
 
}