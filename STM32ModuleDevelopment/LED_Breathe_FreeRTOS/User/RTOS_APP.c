#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "RTOS_APP.h"
#include "stm32f10x.h"
#include "LED.h"

/************************************
	流水灯任务 
************************************/
#define RWLED_Task_STACKDEPTH  128  //可以在后续使用xTaskGetStackHighWaterMark()函数获取指定任务的历史最小剩余内存进行调整
#define RWLED_Task_PRIO         1   //任务优先级
TaskHandle_t* RWLEDTask_Handle;
void Running_Water_LED_Task(void* pvParameters){
	uint8_t Flag = 0x10;
	UBaseType_t Mem; 
	LED_Init(GPIOA, GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2);
	while(1){
		LED_Running_Water_lamps_FOR_RTOS(0,2,&Flag);
		vTaskDelay(500);
		LED_Running_Water_lamps_FOR_RTOS(0,2,&Flag);
		vTaskDelay(500);
		Mem = uxTaskGetStackHighWaterMark(NULL);
	}
}
/************************************
	FreeRTOS开始任务
************************************/
#define Start_Task_STACKDEPTH  128  //可以在后续使用xTaskGetStackHighWaterMark()函数获取指定任务的历史最小剩余内存进行调整
#define Start_Task_PRIO         1   //任务优先级
TaskHandle_t* StartTask_Handle;
void Strat_Task(void* pvParameters){
	taskENTER_CRITICAL();//进入临界区
	//创建流水灯任务
	xTaskCreate((TaskFunction_t) Running_Water_LED_Task,
						(char*) "RW_LED_Task", /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
						(configSTACK_DEPTH_TYPE) RWLED_Task_STACKDEPTH,
						(void*) NULL,
						(UBaseType_t) RWLED_Task_PRIO,
						(TaskHandle_t*) RWLEDTask_Handle);
	//销毁当前任务
	vTaskDelete(NULL);
	taskEXIT_CRITICAL();
}


void FreeRTOS_Init(void){
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//中断分组,仅使用抢占式中断
	xTaskCreate((TaskFunction_t) Strat_Task,
							(char*) "Strat_Task", /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
							(configSTACK_DEPTH_TYPE) Start_Task_STACKDEPTH,
							(void*) NULL,
							(UBaseType_t) Start_Task_PRIO,
							(TaskHandle_t*) StartTask_Handle);
	vTaskStartScheduler();//开启任务调度器
}
