#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "RTOS_APP.h"
#include "stm32f10x.h"
#include "LED.h"

/************************************
	��ˮ������ 
************************************/
#define RWLED_Task_STACKDEPTH  128  //�����ں���ʹ��xTaskGetStackHighWaterMark()������ȡָ���������ʷ��Сʣ���ڴ���е���
#define RWLED_Task_PRIO         1   //�������ȼ�
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
	FreeRTOS��ʼ����
************************************/
#define Start_Task_STACKDEPTH  128  //�����ں���ʹ��xTaskGetStackHighWaterMark()������ȡָ���������ʷ��Сʣ���ڴ���е���
#define Start_Task_PRIO         1   //�������ȼ�
TaskHandle_t* StartTask_Handle;
void Strat_Task(void* pvParameters){
	taskENTER_CRITICAL();//�����ٽ���
	//������ˮ������
	xTaskCreate((TaskFunction_t) Running_Water_LED_Task,
						(char*) "RW_LED_Task", /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
						(configSTACK_DEPTH_TYPE) RWLED_Task_STACKDEPTH,
						(void*) NULL,
						(UBaseType_t) RWLED_Task_PRIO,
						(TaskHandle_t*) RWLEDTask_Handle);
	//���ٵ�ǰ����
	vTaskDelete(NULL);
	taskEXIT_CRITICAL();
}


void FreeRTOS_Init(void){
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//�жϷ���,��ʹ����ռʽ�ж�
	xTaskCreate((TaskFunction_t) Strat_Task,
							(char*) "Strat_Task", /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
							(configSTACK_DEPTH_TYPE) Start_Task_STACKDEPTH,
							(void*) NULL,
							(UBaseType_t) Start_Task_PRIO,
							(TaskHandle_t*) StartTask_Handle);
	vTaskStartScheduler();//�������������
}
