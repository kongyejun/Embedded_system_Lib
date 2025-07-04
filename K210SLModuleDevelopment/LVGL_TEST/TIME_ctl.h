#ifndef __TIME_CTL__
#define __TIME_CTL__
#include "timer.h"
#include "gpio.h"
#include "Pin_Config.h"

/* 
@name   定时器初始化函数
@param  timer_number:   定时器编号
        channel:        定时器通道号
        overtime:       超时时间(ms)
        is_single_shot: 是否是单次触发
        priority:       中断优先级
        callback:       回调函数
        ctx:            回调函数参数
*/
void Timer_Init(timer_device_number_t timer_number,timer_channel_number_t channel,size_t overtime,
                int is_single_shot, uint32_t priority, 
                timer_callback_t callback, void *ctx);
/*
@name   LVGL心跳定时器
*/              
#define LVGL_TICK_TIMERNUM TIMER_DEVICE_0
#define LVGL_TICK_CHANNEL TIMER_CHANNEL_0
#define LVGL_TICK_TIME 1
void LVGL_tick_Init(void);
/*
@name   定时器0通道0的中断函数
*/
int Time0_channel0_ISR(void* ctx);
/*
@name   定时器0通道1的中断函数
*/
int Time0_channel1_ISR(void* ctx);
/*
@name   定时器0通道2的中断函数
*/
int Time0_channel2_ISR(void* ctx);
#endif