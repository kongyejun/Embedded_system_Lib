#include "OLED_I2CPeriph.h"
#include "KEY.h"
#include "Delay.h"
#include "MOTOR_BASE.h"
#include "stm32f10x_gpio.h"

#define MAX_LIFT 7
#define LED_Pin GPIO_Pin_10

int16_t target_floor = 1;
int16_t current_floor= 1;
uint8_t Lift_Floor_state = 0;
int8_t  Lift_Next_floor = -1;
uint8_t FLAG = 0;

void Lift_LEDState(uint8_t state){
    if(state){// 电梯工作 绿灯亮 输出高电平
        GPIO_SetBits(GPIOA,LED_Pin);
    }else{
        GPIO_ResetBits(GPIOA,LED_Pin);
    }
}

void process_event(void){
    switch (KEY_EVENT_FIFO_Pop()){
    case KEY_EVENT_NULL://OLED_ShowString(1,1,"KEY  EVENT  NULL ");
        return;
    case KEY1_REALSE:
        //OLED_ShowString(1,1,"KEY1 REALSE      "); 
        return ;
    case KEY2_REALSE:
        //OLED_ShowString(1,1,"KEY2 REALSE      "); 
        return ;
    case KEY1_PRESS:
        //OLED_ShowString(1,1,"KEY1 PRESS       ");
        target_floor=(target_floor>MAX_LIFT?MAX_LIFT+1:target_floor+1); return;
    case KEY2_PRESS:
        //OLED_ShowString(1,1,"KEY2 PRESS       ");  
        target_floor=(target_floor<=1?1:target_floor-1); return;
    case KEY3_PRESS:
        Lift_Floor_state |=  (0x01 << (target_floor-1)); return;// 设置需要前往的楼层
    case KEY1_LONG_PRESS:
        //OLED_ShowString(1,1,"KEY1 LONG   PRESS"); 
        return;
    case KEY2_LONG_PRESS:
        //OLED_ShowString(1,1,"KEY2 LONG   PRESS"); 
        return;
    case KEY3_LONG_PRESS:
        if((Lift_Floor_state&(~(0x01 << (target_floor-1))))==0)return ;
        Lift_Floor_state &= ~(0x01 << (target_floor-1)); return;// 取消需要前往的楼层
    case KEY1_BURSTS:
        //OLED_ShowString(1,1,"KEY1 BURSTS      "); 
        return;
    case KEY2_BURSTS:
        //OLED_ShowString(1,1,"KEY2 BURSTS      "); 
        return;
    default : return ;
    }
}

void lift_dispose(){
    static uint16_t delay_time = 0;
    if(FLAG==3){
        delay_time+=1;
        if(delay_time>30){
            FLAG = 0;
            delay_time=0;
        }
        return ;
    }
    uint8_t i=current_floor-1;
    OLED_ShowNum(1,15,FLAG,1);
    OLED_ShowSignedNum(1,11,Lift_Next_floor,1);
    if(Lift_Floor_state){
        if(FLAG == 1 || FLAG == 0){
            while(!((0x01<<i)&Lift_Floor_state)){i+=1;i%=MAX_LIFT+1;}
        }else if(FLAG == 2){
            while(!((0x01<<i)&Lift_Floor_state)){i=(i<=0?MAX_LIFT:i-1);}
        }
        if(i>=8){OLED_ShowString(1,1,"Lift ERROR");while(1);}
        Lift_Next_floor = i+1;
        if(current_floor<Lift_Next_floor){
            // OLED_ShowString(1,1,"Lift UP   ");
            FLAG = 1;
            Motor_SetSpeed((((Lift_Next_floor - current_floor)*50)>>3)+40);
            if(delay_time>20){
                current_floor+=1;delay_time=0;
            }
        }else if(current_floor>Lift_Next_floor){
            // OLED_ShowString(1,1,"Lift DOWN ");
            FLAG = 2;
            Motor_SetSpeed((((Lift_Next_floor - current_floor)*50)>>3)-40);
            if(delay_time>20){
                current_floor-=1;delay_time=0;
            }
        }else{
            FLAG = 3;Lift_Next_floor=-1;
            Motor_SetSpeed(0);
            OLED_ShowString(1,1,"Lift STOP ");
            Lift_LEDState(0);// 电梯停止  红灯亮
            Lift_Floor_state &= ~(0x01<<(current_floor-1));
            return ;
        }
        delay_time += 1;
        Lift_LEDState(1);// 电梯工作  绿灯亮
    }
}


void LED_Init(){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = LED_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    Lift_LEDState(0);// 电梯暂停  红灯亮 默认
}


int main(void){
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	OLED_Init();
//    LED_Init();
//    KEY_Init();
//    MOTOR_Init();
//    Key_evnet_t Key_event=KEY_EVENT_NULL;
    OLED_ShowNOWChar16X16(2,1);
    OLED_ShowTargetChar16X16(3,1);
    OLED_ShowString(1,1,"Lift STOP ");
    while(1){        
        //process_event();
        OLED_ShowSignedNum(2,10,current_floor,2);
        OLED_ShowSignedNum(3,10,target_floor,2);
        OLED_ShowBinNum(4,1,Lift_Floor_state,8);
        OLED_ShowChar(4,9-current_floor,'H');
        //lift_dispose();
        Delay_ms(50);
    }
	return 0;
}
