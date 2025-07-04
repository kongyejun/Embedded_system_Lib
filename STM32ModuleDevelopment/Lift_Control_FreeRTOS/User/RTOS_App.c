#include "RTOS_App.h"
#include "FreeRTOS.h"
#include "task.h"
#include "KEY.h"
#include "OLED_I2CPeriph.h"
#include "Delay.h"
#include "MOTOR_BASE.h"
#include "LED.h"

#define MAX_LIFT 7

int16_t target_floor = 1;
int16_t current_floor= 1;
uint8_t Lift_Floor_state = 0;
int8_t  Lift_Next_floor = -1;
uint8_t FLAG = 0;

void process_event(void){
    switch (KEY_EVENT_FIFO_Pop()){
    case KEY1_PRESS:
        target_floor=(target_floor>MAX_LIFT?MAX_LIFT+1:target_floor+1); return;
    case KEY2_PRESS:
        target_floor=(target_floor<=1?1:target_floor-1); return;
    case KEY3_PRESS:
        Lift_Floor_state |=  (0x01 << (target_floor-1)); return;// 设置需要前往的楼层
    case KEY3_LONG_PRESS:
        if((Lift_Floor_state&(~(0x01 << (target_floor-1))))==0)return;
        Lift_Floor_state &= ~(0x01 << (target_floor-1)); return;// 取消需要前往的楼层
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
    if(Lift_Floor_state){
        if(FLAG == 1 || FLAG == 0){
            while(!((0x01<<i)&Lift_Floor_state)){i+=1;i%=MAX_LIFT+1;}
        }else if(FLAG == 2){
            while(!((0x01<<i)&Lift_Floor_state)){i=(i<=0?MAX_LIFT:i-1);}
        }
        if(i>=8){OLED_ShowString(1,1,"Lift ERROR");while(1);}
        Lift_Next_floor = i+1;
        if(current_floor<Lift_Next_floor){
            FLAG = 1;
            Motor_SetSpeed((((Lift_Next_floor - current_floor)*50)>>3)+40);
            if(delay_time>20){
                current_floor+=1;delay_time=0;
            }
        }else if(current_floor>Lift_Next_floor){
            FLAG = 2;
            Motor_SetSpeed((((Lift_Next_floor - current_floor)*50)>>3)-40);
            if(delay_time>20){
                current_floor-=1;delay_time=0;
            }
        }else{
            FLAG = 3;Lift_Next_floor=-1;
            Motor_SetSpeed(0);
            Lift_LEDState(0);// 电梯停止  红灯亮
            Lift_Floor_state &= ~(0x01<<(current_floor-1));
            return ;
        }
        delay_time += 1;
        Lift_LEDState(1);// 电梯工作  绿灯亮
    }
}


/************************************
	        OELD任务 
************************************/
#define OLED_TASK_PRIO   	    1   		//OELD任务优先级
#define OLED_TASK_STKSIZE    	128			//任务堆栈大小
TaskHandle_t OLED_task_handle;			    //任务句柄
void OLED_TASK(void *pvParameters){
    OLED_ShowNOWChar16X16(2,1);
    OLED_ShowTargetChar16X16(3,1);
    OLED_ShowString(1,1,"Lift STOP ");
    while(1){
        OLED_ShowNum(1,15,FLAG,1);
        if(FLAG==0){
            OLED_ShowString(1,1,"Lift STOP ");
        }else if(FLAG==1){
            OLED_ShowString(1,1,"Lift UP   ");
        }else if(FLAG==2){
            OLED_ShowString(1,1,"Lift DOWN ");
        }
        OLED_ShowSignedNum(1,11,Lift_Next_floor,1);
        OLED_ShowSignedNum(2,10,current_floor,2);
        OLED_ShowSignedNum(3,10,target_floor,2);
        OLED_ShowBinNum(4,1,Lift_Floor_state,8);
        OLED_ShowChar(4,9-current_floor,'H');
        vTaskDelay(100);
    }
}

/************************************
	        按键检测任务 
************************************/
#define KEYSCAN_TASK_PRIO   	    3   		//按键扫描任务优先级
#define KEYSCAN_TASK_STKSIZE    	128			//任务堆栈大小
TaskHandle_t KeyScan_task_handle;			    //任务句柄
void KEYSCAN_TASK(void *pvParameters){
    while(1){
        Key_Scan();
        vTaskDelay(20);
    }
}

/************************************
	        按键事务处理任务 
************************************/
#define KEY_PROCESS_EVENT_TASK_PRIO   	    2   		//按键事务处理任务优先级
#define KEY_PROCESS_EVENT_TASK_STKSIZE    	64			//任务堆栈大小
TaskHandle_t Keyprocess_event_task_handle;			    //任务句柄
void Keyprocess_event(void *pvParameters){
    while(1){
        process_event();
        lift_dispose();
        vTaskDelay(50);
    }
}

/************************************
        FreeRTOS开始任务 
************************************/
void SystemStart_Task(void){
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//中断分组,仅使用抢占式中断
    taskENTER_CRITICAL();//进入临界区
    xTaskCreate((TaskFunction_t)OLED_TASK,           // 任务函数的指针
                (char *)        "OLED_TASK",         // 任务的名字（可选）
                (uint16_t)      OLED_TASK_STKSIZE,   // 任务栈的深度（单位：字）
                (void *)        NULL,                // 传递给任务函数的参数（如果不需要参数，可以为 NULL）
                (UBaseType_t)   OLED_TASK_PRIO,      // 任务优先级
                (TaskHandle_t *)&OLED_task_handle    // 返回任务句柄的指针（可选）
    );
    xTaskCreate((TaskFunction_t)KEYSCAN_TASK,           // 任务函数的指针
                (char *)        "KEYSCAN_TASK",         // 任务的名字（可选）
                (uint16_t)      KEYSCAN_TASK_STKSIZE,   // 任务栈的深度（单位：字）
                (void *)        NULL,                   // 传递给任务函数的参数（如果不需要参数，可以为 NULL）
                (UBaseType_t)   KEYSCAN_TASK_PRIO,      // 任务优先级
                (TaskHandle_t *)&KeyScan_task_handle    // 返回任务句柄的指针（可选）
    );
    xTaskCreate((TaskFunction_t)Keyprocess_event,               // 任务函数的指针
                (char *)        "KEY_PROCESS_EVENT_TASK",       // 任务的名字（可选）
                (uint16_t)      KEY_PROCESS_EVENT_TASK_STKSIZE, // 任务栈的深度（单位：字）
                (void *)        NULL,                           // 传递给任务函数的参数（如果不需要参数，可以为 NULL）
                (UBaseType_t)   KEY_PROCESS_EVENT_TASK_PRIO,    // 任务优先级
                (TaskHandle_t *)&Keyprocess_event_task_handle   // 返回任务句柄的指针（可选）
    );
    taskEXIT_CRITICAL();   //退出临界区
    vTaskStartScheduler(); // 启动调度器
    vTaskDelete(NULL);     // 调度器启动后，此任务应该被删除
}
