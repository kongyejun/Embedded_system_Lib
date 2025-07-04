#ifndef __KEY_H__
#define __KEY_H__
#include "stm32f10x.h"
                /****GPIO分区为B区*****/            
#define KEY1_PIN  GPIO_Pin_0
#define KEY2_PIN  GPIO_Pin_1
#define KEY3_PIN  GPIO_Pin_5
// #define KEY3_PIN  GPIO_Pin_12

#define KEY_EVENTS_NUM 4
#define TIME_OVER_MS  20
#define LONG_TIME_MAX 1000
#define BURSTS_TIME_MAX 400
#define KEY_EVENT_FIFO_MAX 25

typedef enum{
    KEY1_ID=0,
    KEY2_ID,
    KEY3_ID,
    KEY_ID_MAX,
}Key_ID_t;

typedef enum{
    KEY1_REALSE,
    KEY1_PRESS,
    KEY1_LONG_PRESS,
    KEY1_BURSTS,

    KEY2_REALSE,
    KEY2_PRESS,
    KEY2_LONG_PRESS,
    KEY2_BURSTS,

    KEY3_REALSE,
    KEY3_PRESS,
    KEY3_LONG_PRESS,
    KEY3_BURSTS,

    KEY_EVENT_NULL,
}Key_evnet_t;

typedef enum{
    REALSE = 0,
    PRESS,
    VALUE_ERR
}KEY_Value_t;

typedef struct {
    uint32_t long_time_max;//长按响应最小时间 ms
    uint32_t long_time_count;
    uint32_t bursts_time_max;// 连击响应时间 ms
    uint32_t bursts_time_count;
    uint8_t state;// 状态
    KEY_Value_t per_value;
    uint8_t allow_event;// 允许触发的事件
}key_struct_t;

extern key_struct_t KEY_Struct[KEY_ID_MAX];
void KEY_Init(void);
void Key_Scan(void);
Key_evnet_t KEY_EVENT_FIFO_Pop(void);
#endif
