#include "GUI\LCD_Button\Button.h"
#include "PIN_Config.h"
#include "gpiohs.h"
#include "System_Bsp.h"
#include "LOG_SYSTEM\LOG.h"

Key_Info_t Key_Info[Key_Num_Max];

int8_t Sreach_DownKey(){
    uint8_t i;
    for(i=0;i<Key_Num_Max;i++){
        if(Key_Info[i].Key_state == KEY_DOWN){
            return i;
        }
    }
    return -1;
}

void KEY_state_up(uint8_t key){
    Key_value_e key_value = (gpiohs_get_pin(KEYPAD_GPIOHSNUM1+key)==0?PRESS:RELEASE);
    Key_Info_t* key_current = &Key_Info[key];
    
    switch (key_current->Key_state){
    case KEY_UP:// 按键释放状态
        if(key_value == PRESS){
            key_current->Key_state = KEY_DOWN_CHECK;// 按键被按下,需进一步确认
        }
        break;
    case KEY_DOWN_CHECK:// 按键按下检测状态
        if(key_value == PRESS){
            key_current->Key_state = KEY_DOWN; // 确认出按键被按下
        }else{
            key_current->Key_state = KEY_UP;   // 按键属于误触发，回退状态
        }
        break;
    case KEY_DOWN:// 按键按下状态
        if(key_value == RELEASE){              
            key_current->Key_state = KEY_UP_CHECK;// 按键被释放,需进一步确认
        }
        break;
    case KEY_UP_CHECK:// 按键释放检测状态
        if(key_value == RELEASE){
            key_current->Key_state = KEY_UP;// 确认出按键被释放
        }else{
            key_current->Key_state = KEY_DOWN;// 按键属于误触发，回退状态
        }
        break;
    default:EMLOG(LOG_ERROR,"按键状态错误,按键号:%d",key+1);while(1);break;
    }
    key_current->Key_value = key_value; // 上一时刻更新按键值
    return ;
}

int Scan_Key(void* ctx){
    uint8_t i;
    for(i=0;i<Key_Num_Max;i++){
        KEY_state_up(i);// 按键状态更新
    }
    return 0;
}

void LCD_Button_Init(){
    // 初始化按键引脚
    gpiohs_set_drive_mode(KEYPAD_GPIOHSNUM1,GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_drive_mode(KEYPAD_GPIOHSNUM2,GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_drive_mode(KEYPAD_GPIOHSNUM3,GPIO_DM_INPUT_PULL_UP);

    // 初始化按键信息结构体
    for(int i=0;i<Key_Num_Max;i++){
        Key_Info[i].Key_state = KEY_UP;
        Key_Info[i].Key_value = RELEASE;
    }
    // 按键扫描定时器初始化
    Timer_Init(TIMER_DEVICE_0,TIMER_CHANNEL_1,20,0,1,Scan_Key,NULL,1);
}
