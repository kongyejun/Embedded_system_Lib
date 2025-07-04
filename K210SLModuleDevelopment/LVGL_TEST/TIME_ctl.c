#include "TIME_ctl.h"
#include "GUI\LVGL\lvgl.h"

int Time0_channel0_ISR(void* ctx){
    lv_tick_inc(LVGL_TICK_TIME);
    return 0;
}

int Time0_channel1_ISR(void* ctx){
    return 0;
}

int Time0_channel2_ISR(void* ctx){
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
    //这里仅仅是初始化Timer，并没有启动
}


void LVGL_tick_Init(){
    Timer_Init(LVGL_TICK_TIMERNUM,LVGL_TICK_CHANNEL,LVGL_TICK_TIME,0,1,Time0_channel0_ISR,NULL);
    timer_set_enable(LVGL_TICK_TIMERNUM, LVGL_TICK_CHANNEL, 1);
}