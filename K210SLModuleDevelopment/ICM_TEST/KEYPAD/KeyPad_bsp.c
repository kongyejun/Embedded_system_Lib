#include "KeyPad_bsp.h"
#include "PIN_Config.h"
#include "memory.h"
#include "timer.h"
#include "LOG.h"

keypad_t keypad[EN_KEY_ID_MAX];
FIFO_t Key_FIFO;
//  获取FIFO 空闲大小
uint8_t Get_FIFO_Free(void){
    uint8_t cont = 0;
    uint8_t R = Key_FIFO.read;
    while(R != Key_FIFO.write){
        ++cont;
        if( (++R) > (FIFO_Max - 1)){
            R = 0;
        }
    }
    return cont;
}
//  按键状态获取底层实现
uint8_t gpiohs_get_pindata(uint8_t pin){
    if(gpiohs_get_pin(pin) == GPIO_PV_HIGH){
        return RELEASE;
    }else{
        return PRESS;
    }
}
//  获取需要检测的按键状态
uint8_t Get_keypad_value(key_id_t key_id){
    if (key_id == EN_KEY_ID_LEFT) {
        return gpiohs_get_pindata(KEYL_GPIOHSNUM);
    } else if (key_id == EN_KEY_ID_RIGHT) {
        return gpiohs_get_pindata(KEYR_GPIOHSNUM);
    } else if (key_id == EN_KEY_ID_MIDDLE) {
        return gpiohs_get_pindata(KEYM_GPIOHSNUM);
    }
    return RELEASE;

}
//  将事件放入FIFO中
void KeyFIFO_push(keypad_event event){
    Key_FIFO.buffer[Key_FIFO.write++] = event;
    if(Key_FIFO.write > (FIFO_Max - 1)){
        Key_FIFO.write = 0;
    }
    EMLOG(LOG_INFO,"KeyFIFO_push %d\n",event);
}
//  从FIFO中读取一个事件
void KeyFIFO_read(keypad_event* event){
    if(Key_FIFO.read == Key_FIFO.write){
        *event = KEY_NONE;
        EMLOG(LOG_ERROR,"%#x,%#x",Key_FIFO.read,Key_FIFO.write);
    }else{
        *event = Key_FIFO.buffer[Key_FIFO.read++];//读取事件,并指针后移!!!!!
        if(Key_FIFO.read > (FIFO_Max - 1)){
            Key_FIFO.read = 0;
        }
        EMLOG(LOG_INFO,"KeyFIFO_read %d\n",*event);
    }
}
//  获取事件
void Get_Keypad_Event(key_id_t key_id){
    uint8_t current_key_value = Get_keypad_value(key_id);
    keypad_t *p_key = &keypad[key_id]; 
    switch(p_key->key_state){
        case KEY_NULL:
            if(current_key_value == PRESS){
                p_key->key_state = KEY_DOWN;
            }
            break;
        case KEY_DOWN:
        EMLOG(LOG_DEBUG,"KEY_DOWN\n");
            if(current_key_value == p_key->key_per_value){
                p_key->key_state = KEY_DOWN_RECHECK;//进入下一状态，进一步检测连击，连发，长按事件
                if(p_key->key_flag & KEY_REPORT_DOWN){//如果需要报告 按下事件
                    KeyFIFO_push((keypad_event)(KEY_STATUS * key_id + 1));//将按下事件放入FIFO中
                }
                if(p_key->key_down_bc != NULL){//如果有按下事件回调函数
                    EMLOG(LOG_DEBUG,"key_down_bc call\n");
                    p_key->key_down_bc(p_key->kdbc_arg);//调用按下事件回调函数,处理事务
                }
            }else{
                p_key->key_state = KEY_NULL;
            }
            break;
        case KEY_DOWN_RECHECK:
            EMLOG(LOG_DEBUG,"KEY_DOWN_RECHECK\n");
            if(current_key_value == p_key->key_per_value){
                if(p_key->long_time > 0){//是否允许长按触发
                //先判断长按，才会有可能触发连发事件
                    // EMLOG(LOG_DEBUG,"p_key->long_count %d\n",p_key->long_count);
                    if(p_key->long_count < p_key->long_time){
                        if((p_key->long_count += KEY_TICKS) >= p_key->long_time){
                            p_key->key_flag |= 0x80; //特殊事件触发标志位,如果触发了长按或者连发事件则不会触发连击事件
                            if(p_key->key_flag & KEY_REPORT_LONG){
                                KeyFIFO_push((keypad_event)(KEY_STATUS * key_id + 3));
                            }
                            if(p_key->long_key_bc != NULL){
                                p_key->long_key_bc(p_key->lkbc_arg);
                            }
                        }
                    }else{
                        if(p_key->bursts_time > 0){
                            // EMLOG(LOG_DEBUG,"p_key->bursts_count %d\n",p_key->bursts_count);
                            if((p_key->bursts_count+= KEY_TICKS) >= p_key->bursts_time){
                                p_key->bursts_count = 0; //连发计数清零
                                p_key->key_flag |= 0x80; //特殊事件触发标志位
                                if(p_key->key_flag & KEY_REPORT_BURSTS){
                                    KeyFIFO_push((keypad_event)(KEY_STATUS * key_id + 4));
                                }
                                if(p_key->bursts_key_bc != NULL){
                                    p_key->bursts_key_bc(p_key->bukbc_arg);
                                }
                            }
                        }
                    }
                }else{EMLOG(LOG_DEBUG,"Don't allow Long Preass\n");}
            }else{
                if((p_key->key_flag & 0x80) == 0x80){
                    //如果成功触发了长按等事件，则清除标志位，并进入等待释放阶段
                    EMLOG(LOG_DEBUG,"KEY_DOWN_RECHECK 0x80\n"),p_key->key_flag &= ~0x80;
                    p_key->key_state = KEY_UP;
                }else{
                    //否则，进入连击检测阶段
                    p_key->key_state = KEY_BATTER;
                }
            }
            break;
        case KEY_BATTER:
        EMLOG(LOG_DEBUG,"KEY_BATTER\n");
            if(current_key_value != p_key->key_per_value){
                EMLOG(LOG_DEBUG,"Batter cont ++\n");
                p_key->batter_count += 1;
                p_key->batter_waittime = 0;
            }else{
                if((p_key->batter_waittime += KEY_TICKS) > p_key->batter_outtime){//连击等待时间超时
                    if(p_key->batter_count <= ((KEY_STATUS - 4) << 1)){//连击数超出则无效
                        if(p_key->key_flag & KEY_REPORT_BATTER){
                            KeyFIFO_push((keypad_event)(KEY_STATUS * key_id + 5 + (p_key->batter_count >> 1)));
                        }
                        if(p_key->batter_key_bc != NULL){
                            p_key->batter_key_bc(p_key->bakbc_arg);//可以通过获取p_key->batter_count来获取按下的次数
                        }
                    }else{
                        EMLOG(LOG_DEBUG,"Batter count over: %d/%d\n",p_key->batter_count,KEY_STATUS);
                    }
                    p_key->batter_waittime = 0;
                    p_key->key_state = KEY_UP;
                }
                EMLOG(LOG_DEBUG,"Batter waittime %d\n",p_key->batter_waittime);
            }
            break;
        case KEY_UP:
        EMLOG(LOG_DEBUG,"KEY_UP\n");
            if(current_key_value == RELEASE && current_key_value == p_key->key_per_value){//等待释放
                if(p_key->key_flag & KEY_REPORT_UP){//如果允许上报 按键释放事件
                    KeyFIFO_push((keypad_event)(KEY_STATUS * key_id + 2));
                }
                if(p_key->key_up_bc != NULL){
                    p_key->key_up_bc(p_key->kubc_arg);
                }
                //初始化相关计数器，便于下一次使用
                p_key->bursts_count = 0;
                p_key->long_count = 0;
                p_key->batter_count = 0;
                p_key->key_state = KEY_NULL;//设置为空闲状态
            }
            break;
        default: break;
    }
    p_key->key_per_value = current_key_value;//保存当前按键值
}
//  扫描按键轮盘
void scan_keypad(void){
    for(int i=0; i< EN_KEY_ID_MAX; i++){
        EMLOG(LOG_DEBUG,"%d key_state: %d\n",i,keypad[i].key_state);
        Get_Keypad_Event((key_id_t)i);
    }
    EMLOG(LOG_DEBUG,"\n\n\n");
}
//  定时器中断入口
int Timer_IRQ(void *ctx){
    scan_keypad();
    return 0;
}
//  按键定时器初始化
void KeyCheck_timer_Init(){
    timer_init(KEY_TIMERNUM);
    timer_set_interval(KEY_TIMERNUM,KEY_TIMER_CHANNEL,KEY_TICKS * 1e6);
    timer_irq_register(KEY_TIMERNUM,KEY_TIMER_CHANNEL,0,1,Timer_IRQ,NULL);
    timer_set_enable(KEY_TIMERNUM,KEY_TIMER_CHANNEL,1);
}
// 按键状态机初始化
void Key_SM_Init(void){
    //初始化GPIO
    gpiohs_set_drive_mode(KEYM_GPIOHSNUM,GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_drive_mode(KEYL_GPIOHSNUM,GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_drive_mode(KEYR_GPIOHSNUM,GPIO_DM_INPUT_PULL_UP);
    //清空FIFO事件栈
    memset(&Key_FIFO,0,sizeof(FIFO_t));
    // 初始化所有按键状态
    for(int i=0;i<EN_KEY_ID_MAX;i++){
        keypad[i].long_time = KEY_LONG_TIME;
        keypad[i].long_count = 0;
        keypad[i].bursts_time = KEY_BURSTS_REPEAT_TIME;
        keypad[i].bursts_count = 0;
        keypad[i].batter_outtime = KEY_BATTER_OUTTIME;
        keypad[i].batter_count = 0;
        keypad[i].batter_waittime = 0;

        keypad[i].long_key_bc = NULL;
        keypad[i].lkbc_arg = NULL;
        keypad[i].bursts_key_bc = NULL;
        keypad[i].bukbc_arg = NULL;
        keypad[i].batter_key_bc = NULL;
        keypad[i].bakbc_arg = NULL;
        keypad[i].key_down_bc = NULL;
        keypad[i].kdbc_arg = NULL;
        keypad[i].key_up_bc = NULL;
        keypad[i].kubc_arg = NULL;
        //仅检测特殊事件
        keypad[i].key_flag|=KEY_REPORT_DOWN;
        keypad[i].key_per_value = GPIO_PV_HIGH;
        keypad[i].key_state = KEY_NULL;
    }
    KeyCheck_timer_Init();
}