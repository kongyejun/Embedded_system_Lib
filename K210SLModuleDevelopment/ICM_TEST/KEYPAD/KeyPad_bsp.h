#ifndef __KEYPAD_BSP_H__
#define __KEYPAD_BSP_H__
#include "gpiohs.h"
#include "timer.h"

/* 按键ID */
typedef enum _key_id_t{
    EN_KEY_ID_LEFT = 0,
    EN_KEY_ID_RIGHT ,
    EN_KEY_ID_MIDDLE ,
    EN_KEY_ID_MAX 
}key_id_t;

typedef struct {
    uint16_t long_time;        // 长按最低时间
    uint16_t long_count;       // 长按计数
    uint16_t  bursts_time;      // 连发周期
    uint16_t  bursts_count;     // 连发计数
    uint16_t  batter_outtime;  // 连击超时
    uint16_t  batter_waittime; // 连击等待时间
    uint8_t  batter_count;     // 连击计数

    uint8_t key_state;         // 按键状态
    uint8_t key_per_value;     // 按键值

    uint8_t key_flag;          // 按键标志

    /* 函数指针*/
    void (*key_down_bc)(void * kdbc_arg);   //按键短按下回调函数
    void *kdbc_arg;                         //按键短按下回调函数参数
    void(*key_up_bc)(void * kubc_arg);      //按键短按抬起回调函数
    void *kubc_arg;                         //按键短按抬起回调函数参数
    void (*long_key_bc)(void *lkbc_arg);    //长按事件回调函数
    void *lkbc_arg;                         //长按事件回调函数参数
    void (*bursts_key_bc)(void *bukbc_arg);  //连发事件回调
    void *bukbc_arg;                         //连发事件回调函数参数
    void (*batter_key_bc)(void *brkbc_arg);  //连击事件回调
    void *bakbc_arg;                         //连击事件回调函数参数
}keypad_t;

#define FIFO_Max 10      // 按键FIFO缓冲大小
typedef struct {
    uint8_t buffer[FIFO_Max];
    uint8_t write;
    uint8_t read;
}FIFO_t;

/* 按键状态机 相关状态*/
typedef enum {
    KEY_NULL = 0,  
    KEY_DOWN,   
    KEY_DOWN_RECHECK,
    KEY_BATTER,
    KEY_UP,
}key_state_t;

/* 定义按键事件*/
typedef enum{
    KEY_NONE = 0,   //表示无按键事件
    // 向左按键事件
    KEY_LEFT_DOWN,
    KEY_LEFT_UP,
    KEY_LEFT_LONG,
    KEY_LEFT_BURSTS,
    KEY_LEFT_BATTER_1,
    KEY_LEFT_BATTER_2,
    KEY_LEFT_BATTER_3,

    // 向右按键事件
    KEY_RIGHT_DOWN,
    KEY_RIGHT_UP,
    KEY_RIGHT_LONG,
    KEY_RIGHT_BURSTS,
    KEY_RIGHT_BATTER_1,
    KEY_RIGHT_BATTER_2,
    KEY_RIGHT_BATTER_3,

    // 中间按键事件
    KEY_MIDDLE_DOWN,
    KEY_MIDDLE_UP,
    KEY_MIDDLE_LONG,
    KEY_MIDDLE_BURSTS,
    KEY_MIDDLE_BATTER_1,
    KEY_MIDDLE_BATTER_2,
    KEY_MIDDLE_BATTER_3,

    KEY_STATUSenmu,
}keypad_event;

#define KEY_STATUS          ((KEY_STATUSenmu-1)/EN_KEY_ID_MAX)// 状态数量
/************************************
 *     按键状态                     *
 ************************************/
#define RELEASE             0              // 按键松开
#define PRESS               1              // 按键按下

/************************************
 *     按键检测定时器/事件触发时间    *
 ************************************/
#define KEY_TIMERNUM  TIMER_DEVICE_0
#define KEY_TIMER_CHANNEL TIMER_CHANNEL_0
/* 修改按键触发时间 */
#define KEY_TICKS           10             // 按键扫描周期（ms）,scan_keypad()函数在哪个固定扫描周期中该值就等于多少 原则上应该是10的公约数中的值 因为按键消抖是10ms
#define KEY_LONG_TIME       500           // 长按触发时间(ms)
#define KEY_BURSTS_REPEAT_TIME     250     // 连发间隔(ms),连发个数(/s)=1000/KEY_REPEAT_TIME
#define KEY_BATTER_OUTTIME  180           // 连击超时时间(ms)

/************************************
 *     事件标志位                    *
 ************************************/
#define KEY_REPORT_DOWN     (1<<0)         // 上报按键按下事件
#define KEY_REPORT_UP       (1<<1)         // 上报按键抬起事件
#define KEY_REPORT_LONG     (1<<2)         // 上报长按事件
#define KEY_REPORT_BURSTS   (1<<3)         // 上报连发事件
#define KEY_REPORT_BATTER   (1<<4)         // 上报连击事件

void Key_SM_Init(void);
void KeyFIFO_read(keypad_event* event);
uint8_t Get_FIFO_Free(void);

#endif