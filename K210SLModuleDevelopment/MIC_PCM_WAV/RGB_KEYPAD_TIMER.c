#include "gpiohs.h"
#include "LOG_SYSTEM/LOG.h"
#include "RGB_KEYPAD_TIMER.h"
/****************************************************************************
 *                            封装定义                                      *
*****************************************************************************/

#define KEYREPORT_DOWN (0x01 << 0)
#define KEYREPORT_UP   (0x01 << 1)

#define KEY_FIFO_SIZE 5
typedef struct{
    uint8_t buff[KEY_FIFO_SIZE];
    uint8_t write;
    uint8_t read;
}FIFO_t;

// 按键键值
typedef enum{
    PRESS = 0,
    RELEASE,
}key_value_t;

// 按键状态机
typedef enum _key_state_t{
    // 表示无按键按下
    KEY_NULL = 0,
    KEY_DOWN,
    KEY_DOWN_RECHECK,
    KEY_UP,
    KEY_UP_RECHECK,
} key_state_t;

typedef struct{
    //状态参数
    uint8_t per_value;
    uint8_t cur_value;
    uint8_t state;
    //上报事件标志
    uint8_t report_flag;    
}keypad_t;

//按键登记
typedef enum _key_id_t{
    KEY_ID_LEFT = 0,
    KEY_ID_RIGHT,
    KEY_ID_MIDDLE,
    KEY_ID_MAX 
}key_id_t;
/****************************************************************************
 *                            全局定义                                      *
*****************************************************************************/

FIFO_t Key_FIFO;
static keypad_t Keypad[3];
/****************************************************************************
 *                            函数实现                                      *
*****************************************************************************/

gpio_pin_value_t _get_key_value(key_id_t* keynum){
    switch(*keynum){
        case KEY_ID_LEFT:return gpiohs_get_pin(KEYPAD_LEFT_GPIONUM);
        case KEY_ID_RIGHT:return gpiohs_get_pin(KEYPAD_RIGHT_GPIONUM);
        case KEY_ID_MIDDLE:return gpiohs_get_pin(KEYPAD_MIDDLE_GPIONUM);
        default: break;
    }
    EMLOG(LOG_ERROR,"keynum not found\n");
    return 0;
}

key_value_t Get_key_Value(key_id_t keynum){
    if(_get_key_value(&keynum)==GPIO_PV_LOW){//低电平为按下
        return PRESS;
    }else{
        return RELEASE;
    }
}

void FIFO_Push(key_event_t event){
    Key_FIFO.buff[Key_FIFO.write] = event;
    Key_FIFO.write += 1;
    if(Key_FIFO.write >= KEY_FIFO_SIZE){
        Key_FIFO.write = 0;
    }
    if(Key_FIFO.read == Key_FIFO.write){    //缓冲区满,则舍弃旧的事件
        Key_FIFO.read += 1;
        if(Key_FIFO.read >= KEY_FIFO_SIZE){ //溢出处理
            Key_FIFO.read = 0;
        }
    }
}

key_event_t FIFO_Pop(void){
    if(Key_FIFO.read == Key_FIFO.write){
        // EMLOG(LOG_DEBUG,"key event None!\n");
        return KEY_EVENT_NONE;
    }
    key_event_t event;
    event = Key_FIFO.buff[Key_FIFO.read];
    Key_FIFO.read += 1;
    if(Key_FIFO.read >= KEY_FIFO_SIZE){
        Key_FIFO.read = 0;
    }
    //EMLOG(LOG_DEBUG,"key event:%d\n",event);
    return event;
}

void Detect_KEY(uint8_t keynum){
    keypad_t* key = &(Keypad[keynum]);
    /*  获取按键状态:
        KEY_NULL = 0,表示无按键按下
        KEY_DOWN,
        KEY_DOWN_RECHECK,
        KEY_UP                    */
    switch(key->state){
        case KEY_NULL:
            if(key->cur_value == PRESS && key->per_value == RELEASE){
                key->state = KEY_DOWN;
            }
            break;
        case KEY_DOWN:
            if(key->cur_value == key->per_value){
                key->state = KEY_DOWN_RECHECK;
                if(key->report_flag & KEYREPORT_DOWN){//如果允许按键按下事件上报
                    EMLOG(LOG_DEBUG,"key down event, Key:%d\n",keynum);
                    FIFO_Push(keynum*KEY_EVENT_CONT + 1);
                }
            }else{
                key->state = KEY_NULL;
            }
            break;
        case KEY_DOWN_RECHECK:
            if(key->cur_value == key->per_value){//处理长按事件
                ;
            }else{//当前键值为松手状态
                key->state = KEY_UP;//进入松手检测状态
            }
            break;
        case KEY_UP:
            if(key->cur_value == RELEASE){
                key->state = KEY_NULL;
                if(key->report_flag & KEYREPORT_UP){//如果允许按键松手事件上报
                    //EMLOG(LOG_DEBUG,"key up event\n");
                    FIFO_Push(keynum*KEY_EVENT_CONT + 2);
                }
            }else{
                key->state = KEY_DOWN_RECHECK;//当前键值为按下状态,说明是误判则退回上一个状态
            }
            break;
        default: break;
    }
    key->per_value = key->cur_value;
    //根据上一个状态进入对应状态区，进行检测
}

void Key_Scan(void){
    for(uint8_t i = 0;i < KEY_ID_MAX;i++){
        //获取按键当前键值
        Keypad[i].cur_value = Get_key_Value((key_id_t)i);//决定Keypad中存放的顺序 
        // EMLOG(LOG_DEBUG,"Get Key value:%d\n",Keypad[i].cur_value);
        Detect_KEY((key_id_t)i);
    }
}

int KEY_Timer_CallBack(void* ctx){
    Key_Scan();
    return 0;
}

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
                timer_callback_t callback, void *ctx){
    if(timer_init_flag != (0x01 << timer_number)){
        EMLOG(LOG_INFO,"Timer %d is not init, So now Init\n",timer_number);
        timer_init(timer_number);
        timer_init_flag |= 0x01 << timer_number;
    }
    timer_set_interval(timer_number,channel,overtime * 1e6);
    timer_irq_register(timer_number,channel,is_single_shot,priority,callback,ctx);
    timer_set_enable(timer_number,channel,1);
}

void RGB_OFF(void){
    //设置对应的默认电平
    gpiohs_set_pin(RGB_R_GPIONUM,GPIO_PV_HIGH);//RGB低电平导通
    gpiohs_set_pin(RGB_G_GPIONUM,GPIO_PV_HIGH);
    gpiohs_set_pin(RGB_B_GPIONUM,GPIO_PV_HIGH);
}

void RGB_R_State(rgb_state_t state){
    if(state == LIGHT_ON){
        gpiohs_set_pin(RGB_R_GPIONUM,GPIO_PV_LOW);
    }else{
        gpiohs_set_pin(RGB_R_GPIONUM,GPIO_PV_HIGH);
    }
}

void RGB_G_State(rgb_state_t state){
    if(state == LIGHT_ON){
        gpiohs_set_pin(RGB_G_GPIONUM,GPIO_PV_LOW);
    }else{
        gpiohs_set_pin(RGB_G_GPIONUM,GPIO_PV_HIGH);
    }
}

void RGB_B_State(rgb_state_t state){
    if(state == LIGHT_ON){
        gpiohs_set_pin(RGB_B_GPIONUM,GPIO_PV_LOW);
    }else{
        gpiohs_set_pin(RGB_B_GPIONUM,GPIO_PV_HIGH);
    }
}

void RGB_KEYPAD_Init(void){
    EMLOG(LOG_DEBUG,"RGB_KEYPAD_Init Start!\n");
    //KEYPAD设置为上拉输入
    gpiohs_set_drive_mode(KEYPAD_LEFT_GPIONUM,GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_drive_mode(KEYPAD_MIDDLE_GPIONUM,GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_drive_mode(KEYPAD_RIGHT_GPIONUM,GPIO_DM_INPUT_PULL_UP);
    EMLOG(LOG_DEBUG,"KEYPAD SET UP INPUT OK!\n");
    //RGB设置为输出
    gpiohs_set_drive_mode(RGB_R_GPIONUM,GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(RGB_G_GPIONUM,GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(RGB_B_GPIONUM,GPIO_DM_OUTPUT);
    EMLOG(LOG_DEBUG,"RGB SET OUTPUT OK!\n");
    //初始化RGB
    RGB_OFF();
    EMLOG(LOG_DEBUG,"RGB Light init OK!\n");

    //初始化FIFO
    Key_FIFO.write = 0;Key_FIFO.read = 0;
    //初始化按键结构体
    for(int i = 0;i < KEY_ID_MAX;i++){
        Keypad[i].per_value=RELEASE;
        Keypad[i].state=KEY_NULL;//初始按键状态为无
        Keypad[i].report_flag = KEYREPORT_DOWN | KEYREPORT_UP;//设置可上报的事件
    }
    EMLOG(LOG_DEBUG,"Keypad struct init OK!\n");

    //定时器初始化
    Timer_Init(KEYPAD_TIMERNUM,KEYPAD_TIMERCHANNEL,KEYPAD_OVERTIMER,0,KEYPAD_TIMERPRIORITY,KEY_Timer_CallBack,NULL);
    EMLOG(LOG_DEBUG,"Key Scan Timer init OK!\n");
}
