#ifndef __BUTTON_H__
#define __BUTTON_H__
#include <stdint.h>
#define Key_Num_Max  3 // 按键数量

typedef enum _Key_value_e{
    PRESS,  // 按下
    RELEASE,// 释放
}Key_value_e;

typedef enum _Key_state_e{
    KEY_NONE      ,  // 无状态
    KEY_DOWN_CHECK,  // 按键按下检测状态
    KEY_DOWN      ,  // 按键按下状态
    KEY_UP_CHECK  ,  // 按键释放检测状态
    KEY_UP        ,  // 按键释放状态
}Key_state_t;

typedef struct _Key_Info_t{
    Key_state_t Key_state;
    Key_value_e Key_value;
}Key_Info_t;

void LCD_Button_Init(void);
int8_t Sreach_DownKey(void);
#endif