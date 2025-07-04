#ifndef __MY_GUI_H__
#define __MY_GUI_H__
#include "GUI\LVGL\lvgl.h"
#define LVGL_TICK_TIMERNUM TIMER_DEVICE_0
#define LVGL_TICK_CHANNEL TIMER_CHANNEL_0
#define LVGL_TICK_TIME 5


int GUI_Init(void);
#endif // __MY_GUI_H__
