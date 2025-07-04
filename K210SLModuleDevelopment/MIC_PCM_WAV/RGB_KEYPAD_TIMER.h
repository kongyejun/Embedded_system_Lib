#ifndef __RGB_KEYPAD_H__
#define __RGB_KEYPAD_H__
#include "PIN_Config.h"
#include "timer.h"

#define KEYPAD_TIMERNUM  TIMER_DEVICE_0
#define KEYPAD_TIMERCHANNEL TIMER_CHANNEL_0
#define KEYPAD_OVERTIMER 20
#define KEYPAD_TIMERPRIORITY 4 

//定义按键事件
#define KEY_EVENT_CONT 2  //按键事件数量
typedef enum{
    // 表示无按键事件
    KEY_EVENT_NONE = 0,
    // 向左按键事件
    KEY_EVENT_LEFT_DOWN,
    KEY_EVENT_LEFT_UP,
    // 向右按键事件
    KEY_EVENT_RIGHT_DOWN,
    KEY_EVENT_RIGHT_UP,
    // 中间按键事件
    KEY_EVENT_MIDDLE_DOWN,
    KEY_EVENT_MIDDLE_UP,
}key_event_t;

typedef enum _rgb_state_t{
    LIGHT_OFF = 0,
    LIGHT_ON,
}rgb_state_t;


key_event_t FIFO_Pop(void);
void RGB_KEYPAD_Init(void);
void RGB_R_State(rgb_state_t state);
void RGB_G_State(rgb_state_t state);
void RGB_B_State(rgb_state_t state);
void RGB_OFF();
#endif