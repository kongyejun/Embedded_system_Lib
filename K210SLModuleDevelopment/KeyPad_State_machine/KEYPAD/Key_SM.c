#include "Key_SM.h"
#include "gpiohs.h"
#include "../PIN_Config.h"
#include "memory.h"
#include "timer.h"
#include "LOG.h"
keypad_t keypad[EN_KEY_ID_MAX];
FIFO_t Key_FIFO;

uint8_t Get_FIFO_Free(void){
    uint8_t cont = 0;
    uint8_t R = Key_FIFO.read;
    while(Key_FIFO.buffer[R] != Key_FIFO.buffer[Key_FIFO.write]){
        ++cont;
        if( (++R) > (FIFO_Max - 1)){
            R = 0;
        }
    }
    return cont;
}

uint8_t gpiohs_get_pindata(uint8_t pin){
    if(gpiohs_get_pin(pin) == GPIO_PV_HIGH){
        return RELEASE;
    }else{
        return PRESS;
    }
}

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

void KeyFIFO_push(keypad_event event){
    Key_FIFO.buffer[Key_FIFO.write++] = event;
    if(Key_FIFO.write > (FIFO_Max - 1)){
        Key_FIFO.write = 0;
    }
}

keypad_event KeyFIFO_read(void){
    keypad_event event;
    if(Key_FIFO.read == Key_FIFO.write){
        return KEY_NONE;
    }else{
        event = Key_FIFO.buffer[Key_FIFO.read++];
        if(Key_FIFO.read > (FIFO_Max - 1)){
            Key_FIFO.read = 0;
        }
    }
    return event;
}

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
                p_key->key_state = KEY_DOWN_RECHECK;
                if(p_key->key_flag & KEY_REPORT_DOWN){
                    EMLOG(LOG_DEBUG,"FIFO push\n");
                    KeyFIFO_push((keypad_event)(KEY_STATUS * key_id + 1));
                }
                if(p_key->key_down_bc != NULL){
                    EMLOG(LOG_DEBUG,"key_down_bc call\n");
                    p_key->key_down_bc(p_key->kdbc_arg);
                }
            }else{
                p_key->key_state = KEY_NULL;
            }
            break;
        case KEY_DOWN_RECHECK:
            EMLOG(LOG_DEBUG,"KEY_DOWN_RECHECK\n");
            if(current_key_value == p_key->key_per_value){
                if(p_key->long_time > 0){
                //先判断长按，在判断连发
                    // EMLOG(LOG_DEBUG,"p_key->long_count %d\n",p_key->long_count);
                    if(p_key->long_count < p_key->long_time){
                        if((p_key->long_count += KEY_TICKS) >= p_key->long_time){
                            p_key->key_flag |= 0x80; //特殊事件触发标志位,如果触发了长按或者连发事件则不会触发连击事件
                            if(p_key->key_flag & KEY_REPORT_LONG){
                                EMLOG(LOG_DEBUG,"FIFO push\n");
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
                                p_key->bursts_count = 0;
                                p_key->key_flag |= 0x80; //特殊事件触发标志位
                                if(p_key->key_flag & KEY_REPORT_BURSTS){
                                    EMLOG(LOG_DEBUG,"FIFO push\n");
                                    KeyFIFO_push((keypad_event)(KEY_STATUS * key_id + 4));
                                }
                                if(p_key->bursts_key_bc != NULL){
                                    p_key->bursts_key_bc(p_key->bukbc_arg);
                                }
                            }
                        }
                    }
                }
            }else{
                if((p_key->key_flag & 0x80) == 0x80){
                    EMLOG(LOG_DEBUG,"KEY_DOWN_RECHECK 0x80\n"),p_key->key_flag &= ~0x80;
                    p_key->key_state = KEY_UP;
                }else{
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
                if((p_key->batter_waittime += KEY_TICKS) > p_key->batter_outtime){
                    if(p_key->batter_count <= ((KEY_STATUS - 4) << 1)){
                        if(p_key->key_flag & KEY_REPORT_BATTER){
                            EMLOG(LOG_DEBUG,"FIFO push\n");
                            KeyFIFO_push((keypad_event)(KEY_STATUS * key_id + 5 + (p_key->batter_count >> 1)));
                        }
                        if(p_key->batter_key_bc != NULL){
                            p_key->batter_key_bc(p_key->bakbc_arg);//可以通过获取p_key->batter_count来获取按下的次数
                        }
                    }
                    p_key->batter_waittime = 0;
                    p_key->key_state = KEY_UP;
                }
                EMLOG(LOG_DEBUG,"Batter waittime %d\n",p_key->batter_waittime);
            }
            break;
        case KEY_UP:
        EMLOG(LOG_DEBUG,"KEY_UP\n");
            if(current_key_value == RELEASE && current_key_value == p_key->key_per_value){
                if(p_key->key_flag & KEY_REPORT_UP){
                    KeyFIFO_push((keypad_event)(KEY_STATUS * key_id + 2));
                }
                if(p_key->key_up_bc != NULL){
                    p_key->key_up_bc(p_key->kubc_arg);
                }
                p_key->bursts_count = 0;
                p_key->long_count = 0;
                p_key->batter_count = 0;
                p_key->key_state = KEY_NULL;
            }
            break;
        default: break;
    }
    p_key->key_per_value = current_key_value;
}

void scan_keypad(void){
    for(int i=0; i< EN_KEY_ID_MAX; i++){
        EMLOG(LOG_DEBUG,"%d key_state: %d\n",i,keypad[i].key_state);
        Get_Keypad_Event((key_id_t)i);
    }
    EMLOG(LOG_DEBUG,"\n\n\n");
}

int Timer_IRQ(void *ctx){
    scan_keypad();
    return 0;
}

void KeyCheck_timer_Init(){
    timer_init(TIMER_DEVICE_0);
    timer_set_interval(TIMER_DEVICE_0,TIMER_CHANNEL_0,KEY_TICKS * 1e6);
    timer_irq_register(TIMER_DEVICE_0,TIMER_CHANNEL_0,0,1,Timer_IRQ,NULL);
    timer_set_enable(TIMER_DEVICE_0,TIMER_CHANNEL_0,1);
}

void Key_SM_Init(void){
    gpiohs_set_drive_mode(KEYM_GPIOHSNUM,GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_drive_mode(KEYL_GPIOHSNUM,GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_drive_mode(KEYR_GPIOHSNUM,GPIO_DM_INPUT_PULL_UP);

    memset(&Key_FIFO,0,sizeof(FIFO_t));

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
        keypad[i].key_flag|=KEY_REPORT_LONG+KEY_REPORT_BURSTS+KEY_REPORT_BATTER;
        keypad[i].key_per_value = GPIO_PV_HIGH;
        keypad[i].key_state = KEY_NULL;
    }
    KeyCheck_timer_Init();
}