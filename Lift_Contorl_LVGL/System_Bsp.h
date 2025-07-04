#ifndef __SYSTEM_BSP_H__
#define __SYSTEM_BSP_H__
#include "timer.h"

void Timer_Init(timer_device_number_t timer_number,timer_channel_number_t channel,size_t overtime,
                int is_single_shot,uint32_t priority, timer_callback_t callback, void *ctx,uint8_t is_enable);

#endif
