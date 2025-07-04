#include "KEY.h"
#include "stm32f10x_tim.h"
#include "OLED_I2CPeriph.h"

#define EVENT_REALSE    (0x01<<0)
#define EVENT_PRESS     (0x01<<1)
#define EVENT_LONGPRESS (0x01<<2)
#define EVENT_BURSTS    (0x01<<3)

typedef enum{
    KEY_NONE,
    KEY_REALSE,
    KEY_PRESS,
    KEY_LONG_PRESS,
}Key_state_t;

typedef struct{
    Key_evnet_t BUFF[KEY_EVENT_FIFO_MAX];
    uint8_t read;
    uint8_t write;
}KEY_EVENT_FIFO_t;

key_struct_t KEY_Struct[KEY_ID_MAX];
KEY_EVENT_FIFO_t KEY_FIFO={0};

void EVENT_FIFO_Push(Key_evnet_t event){
    KEY_FIFO.BUFF[KEY_FIFO.write] = event;
    KEY_FIFO.write++;
    // 边界处理
    if(KEY_FIFO.write >= KEY_EVENT_FIFO_MAX){
        KEY_FIFO.write = 0;
    }
    // 栈满处理
    if(KEY_FIFO.write == KEY_FIFO.read){
        KEY_FIFO.read++;
        if(KEY_FIFO.read >= KEY_EVENT_FIFO_MAX){
            KEY_FIFO.read = 0;
        }
    }
}

Key_evnet_t KEY_EVENT_FIFO_Pop(void){
    if(KEY_FIFO.read == KEY_FIFO.write){// 栈空处理
        return KEY_EVENT_NULL;
    }else{
        Key_evnet_t event = KEY_FIFO.BUFF[KEY_FIFO.read];
        KEY_FIFO.read++;
        // 边界处理
        if(KEY_FIFO.read >= KEY_EVENT_FIFO_MAX){
            KEY_FIFO.read = 0;
        }
        return event;
    }
}

KEY_Value_t Get_GPIO_Value(uint16_t GPIO_PIN){
    if(!GPIO_ReadInputDataBit(GPIOB, GPIO_PIN)){// 此处使用GPIOB分区
        return PRESS;
    }else{
        return REALSE;
    }
}

KEY_Value_t Get_KEY_value(Key_ID_t key_id){
    if(key_id == KEY1_ID){
        return Get_GPIO_Value(KEY1_PIN);
    }else if(key_id == KEY2_ID){
        return Get_GPIO_Value(KEY2_PIN);
    }else if(key_id == KEY3_ID){
        return Get_GPIO_Value(KEY3_PIN);
    }else{
        while(1);// 错误处理
        return VALUE_ERR;
    }
}

void decate_state(Key_ID_t key_id, KEY_Value_t curr_value){
    switch(KEY_Struct[key_id].state){
        case KEY_NONE:
            if(curr_value == PRESS && KEY_Struct[key_id].per_value == REALSE){// 检测是否处于按下状态
                KEY_Struct[key_id].state = KEY_PRESS;
            }
            break;
        case KEY_PRESS:
            if(curr_value == PRESS){// 再次检测是否处于按下状态，以防止误操作
                if(KEY_Struct[key_id].allow_event & EVENT_PRESS){
                    EVENT_FIFO_Push(key_id*KEY_EVENTS_NUM + 1);
                }
                KEY_Struct[key_id].state = KEY_LONG_PRESS;
            }else{
                KEY_Struct[key_id].state = KEY_NONE;// 错误判断,退回到上一个状态
            }
            break;
        case KEY_LONG_PRESS:
            if(curr_value == PRESS){
                if(KEY_Struct[key_id].long_time_max > 0){
                    // 长按检测
                    if(KEY_Struct[key_id].long_time_count < KEY_Struct[key_id].long_time_max){// 如果未达到长按触发时间，则继续计时
                        KEY_Struct[key_id].long_time_count+=TIME_OVER_MS;// 计时
                        // 如果超过最低长按出发时间，则触发长按事件
                        if(KEY_Struct[key_id].long_time_count >= KEY_Struct[key_id].long_time_max){
                            if(KEY_Struct[key_id].allow_event & EVENT_LONGPRESS){// 如果允许触发长按事件
                                EVENT_FIFO_Push(key_id*KEY_EVENTS_NUM + 2);
                            }
                        }
                    }else{// 连发检测，只有长按后才会触发连发
                        if(KEY_Struct[key_id].bursts_time_max > 0){
                            KEY_Struct[key_id].bursts_time_count+=TIME_OVER_MS;// 计时
                            if(KEY_Struct[key_id].bursts_time_max <= KEY_Struct[key_id].bursts_time_count){
                                if(KEY_Struct[key_id].allow_event & EVENT_BURSTS){// 如果允许触发连发事件
                                    EVENT_FIFO_Push(key_id*KEY_EVENTS_NUM + 3);   
                                }
                                KEY_Struct[key_id].bursts_time_count=0;// 重置连发计时
                            }
                        }
                    }
                }
            }else{// 从长按中松开按键
                KEY_Struct[key_id].state = KEY_REALSE;
            }
            break;
        case KEY_REALSE:
            if(curr_value == REALSE){
                if(KEY_Struct[key_id].allow_event & EVENT_REALSE){// 如果允许触发松开事件
                    EVENT_FIFO_Push(key_id*KEY_EVENTS_NUM + 0);
                }
                KEY_Struct[key_id].bursts_time_count = 0;
                KEY_Struct[key_id].long_time_count = 0;
                KEY_Struct[key_id].state = KEY_NONE;// 重新进入空闲状态
            }else{
                KEY_Struct[key_id].state = KEY_LONG_PRESS;// 误判，重新进入长按状态
            }
            break;
    }
    KEY_Struct[key_id].per_value = curr_value;
}

void Key_Scan(void) {
    static uint8_t i;
    static KEY_Value_t curr_value;
    for(i=0;i<KEY_ID_MAX;i++){
        curr_value = Get_KEY_value((Key_ID_t)i);
        decate_state((Key_ID_t)i,curr_value);
    }
}

void Scan_Key_TimerInit(uint32_t period, uint32_t prescaler) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // 使能TIM2时钟
    TIM_InternalClockConfig(TIM2);// 使用内部72MHZ时钟源

    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = period - 1;//设定自动重装载计数值    1000   1ms
    TIM_TimeBaseStructure.TIM_Prescaler = prescaler - 1;//设定预分频系数    72
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//不进行与分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//设置计数模式为向上计数
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);//初始化TIM2

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn; // 定时器中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; // 使能中断
    NVIC_Init(&NVIC_InitStructure);

    TIM_ClearITPendingBit(TIM2, TIM_IT_Update); // 清除中断标志
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); // 使能更新中断
    TIM_Cmd(TIM2, ENABLE); // 启动定时器
}

void KEY_Event_Init(void){
    uint8_t i;
    for(i=0;i<KEY_ID_MAX;i++){// 初始化按键结构体
        KEY_Struct[i].allow_event = EVENT_REALSE|EVENT_PRESS|EVENT_LONGPRESS|EVENT_BURSTS;
        KEY_Struct[i].state = KEY_NONE;
        KEY_Struct[i].long_time_count = 0;
        KEY_Struct[i].long_time_max = LONG_TIME_MAX;
        KEY_Struct[i].bursts_time_count = 0;
        KEY_Struct[i].bursts_time_max = BURSTS_TIME_MAX;
        KEY_Struct[i].per_value = REALSE;
    }
    KEY_FIFO.write=0;KEY_FIFO.read=0;// 清空FIFO
}

void KEY_Init(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    // 初始化按键引脚
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin=KEY1_PIN|KEY2_PIN|KEY3_PIN;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    KEY_Event_Init();
    Scan_Key_TimerInit(TIME_OVER_MS*10,7200);
}

void TIM2_IRQHandler(void) {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_Cmd(TIM2, DISABLE); // 启动定时器
        Key_Scan();
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update); // 清除中断标志
        TIM_Cmd(TIM2, ENABLE); // 启动定时器
    }
}