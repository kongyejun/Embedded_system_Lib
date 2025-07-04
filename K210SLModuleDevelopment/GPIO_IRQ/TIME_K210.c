#include "TIME_K210.h"

int Time0_channel0_ISR(void* ctx){
    static int cont0=0;
    if(cont0){
        gpio_set_pin(RED_LED_GPIONUM,GPIO_PV_HIGH);
    }else{
        gpio_set_pin(RED_LED_GPIONUM,GPIO_PV_LOW);
    }
    cont0=(cont0 + 1) % 2;
    timer_set_enable(TIMER_DEVICE_0,TIMER_CHANNEL_0,0);
    return 0;
}

int Time0_channel1_ISR(void* ctx){
    static int cont1=0;
    if(cont1){
        gpio_set_pin(RED_LED_GPIONUM,GPIO_PV_HIGH);
        gpio_set_pin(GREEN_LED_GPIONUM,GPIO_PV_HIGH);
    }else{
        gpio_set_pin(RED_LED_GPIONUM,GPIO_PV_LOW);
        gpio_set_pin(GREEN_LED_GPIONUM,GPIO_PV_LOW);
    }
    cont1=(cont1 + 1) % 2;
    timer_set_enable(TIMER_DEVICE_0,TIMER_CHANNEL_1,0);
    return 0;
}

int Time0_channel2_ISR(void* ctx){
    static int cont2=0;
    if(cont2){
        gpio_set_pin(GREEN_LED_GPIONUM,GPIO_PV_HIGH);
    }else{
        gpio_set_pin(GREEN_LED_GPIONUM,GPIO_PV_LOW);
    }
    cont2=(cont2 + 1) % 2;
    timer_set_enable(TIMER_DEVICE_0,TIMER_CHANNEL_2,0);
    return 0;
}


void Timer_Init(timer_device_number_t timer_number,timer_channel_number_t channel,size_t overtime,
                int is_single_shot, uint32_t priority, 
                timer_callback_t callback, void *ctx){
    static timer_device_number_t old_timer_num = TIMER_DEVICE_MAX;
    /*
        一定要注意每一次初始化都会导致通道中的中断函数被清除绑定，
        所以若要采用这种写法一定要避免被重复初始化！！！！！！！！！！！！！！！
    */
    if(old_timer_num != timer_number){
        timer_init(timer_number);
        old_timer_num = timer_number;
    }
    timer_set_interval(timer_number,channel,overtime * 1e6);
    timer_irq_register(timer_number,channel,is_single_shot,priority,callback,ctx);
    timer_set_enable(timer_number,channel,0);
}
