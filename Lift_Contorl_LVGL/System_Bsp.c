#include "System_Bsp.h"
#include "LOG_SYSTEM\LOG.h"

/************************************************
 *                 定时器部分
 ************************************************/
/* 
作用: 定时器占用标志位
描述: [0~3] - 定时器0通道0~3;[4~7] - 定时器1通道0~3;[8~11] - 定时器2通道0~3;
      [12~14] - 定时器设备是否使用标志位
*/
uint16_t Timer_use_flag = 0;

/**
 * @name   定时器初始化函数（3定时器4通道共12个定时模块可选）
 * @param  timer_number 定时器编号 - TIMER_DEVICE_0,TIMER_DEVICE_1,TIMER_DEVICE_2,
 * @param  channel 定时器通道      - TIMER_CHANNEL_0,TIMER_CHANNEL_1,TIMER_CHANNEL_2,TIMER_CHANNEL_3,
 * @param  overtime 定时器溢出时间  
 * @param  is_single_shot 是否单次触发
 * @param  priority 定时器优先级   - 1-1023 可用优先级范围
 * @param  callback 定时器回调函数
 * @param  ctx 定时器上下文
 * @param  is_enable 是否使能定时器
 * @retval None
 */
void Timer_Init(timer_device_number_t timer_number,timer_channel_number_t channel,size_t overtime,
                int is_single_shot,uint32_t priority, timer_callback_t callback, void *ctx,uint8_t is_enable){  
    // 定时器初始化检查
    if(!(Timer_use_flag & (0x1000<<timer_number))){
        timer_init(timer_number);//一定要注意每一次初始化都会导致通道中的中断函数被清除绑定
        Timer_use_flag |=  (0x1000<<timer_number);
    }
    // 定时器通道占用检查
    if(Timer_use_flag & (0x0001<<(4*timer_number+channel))){// 如果已占用,报错
        EMLOG(LOG_ERROR,"定时器%d通道%d已被使用\n",timer_number,channel);
        while(1);
    }
    Timer_use_flag |= (0x0001<<(4*timer_number+channel));// 如果未占用,则设置占用

    timer_set_interval(timer_number,channel,overtime * 1e6);
    timer_irq_register(timer_number,channel,is_single_shot,priority,callback,ctx);
    timer_set_enable(timer_number,channel,is_enable);
}

