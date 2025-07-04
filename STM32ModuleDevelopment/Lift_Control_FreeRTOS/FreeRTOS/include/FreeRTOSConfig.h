#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H
// =====================
// CPU�Ͷ�ջ����
// =====================
#define configCPU_CLOCK_HZ         ((unsigned long)72000000)  // CPUʱ��Ƶ������Ϊ72MHz
#define configTICK_RATE_HZ         ((TickType_t)1000)         // ϵͳ�δ�Ƶ������Ϊÿ��1000�Σ�1ms��
#define configMINIMAL_STACK_SIZE   ((unsigned short)128)      // ÿ���������С��ջ��С����Ϊ128�ֽ�
#define configTOTAL_HEAP_SIZE      ((size_t)(10*1024))       // �ܶ��ڴ��С����Ϊ10KB
#define configMAX_TASK_NAME_LEN    (16)                       // �������Ƶ���󳤶�����Ϊ16���ַ�
#define configTIMER_TASK_STACK_DEPTH (configMINIMAL_STACK_SIZE*2) // �����ʱ�������ջ��С
#define configUSE_IDLE_HOOK         0                         // ���ÿ��й��Ӻ��������й�����ϵͳ����ʱ����
#define configUSE_TICK_HOOK         0                         // ���õδ��Ӻ������δ�����ÿ��ϵͳ�δ�ʱ���е���

// =====================
// ����͵������������
// =====================
#define configUSE_PREEMPTION        1                         // ������ռʽ���ȣ���������ȼ������жϵ����ȼ�����
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1             // ����Ӳ��������һ����(ʵ������ʹ����Ӳ��ƽ̨������ָ��),�����������һ�������Ч
#define configMAX_PRIORITIES       (32)                       // ����������ȼ���������Ϊ32
/*
  Ϊʲô��32λ����Ϊ"configUSE_PORT_OPTIMISED_TASK_SELECTION"������Ӳ��������һ����ķ�ʽ,
  ��STM32֧�����ַ�ʽ,����Ϊ��32λ��ԭ��,����߽�֧��32(0~31)������ȼ�
*/
#define configUSE_TRACE_FACILITY    1                         // ���ø��ٹ��ܣ������¼���������ʱ�����Ϣ
#define configUSE_16_BIT_TICKS      0                         // ����16λ�δ������ʹ��32λ�δ����
#define configUSE_QUEUE_SETS        1                         // ���ö��м���
#define configUSE_TASK_NOTIFICATIONS 1                        // ��������֪ͨ����
#define configUSE_MUTEXES           1                         // ʹ�û����ź���
#define configUSE_RECURSIVE_MUTEXES 1                         // ʹ�õݹ黥���ź���
#define configUSE_COUNTING_SEMAPHORES 1                       // ���ü����ź���
#define configQUEUE_REGISTRY_SIZE   10                        // ���ÿ���ע����ź�������Ϣ���и���
#define configUSE_TIME_SLICING      1                         // ����ʱ��Ƭ���ȣ�����������ʱ��Ƭ����ʱ�����л�
#define configIDLE_SHOULD_YIELD     1                         // ��������Ӧ���ó�CPU������������������

// =====================
// �ж����ȼ�����ϵͳ�жϽӿ�
// =====================

/*  ��STM32ʹ�������ж����ȼ����ò��õ��ǿ⺯�����������뱣֤���ȼ�����Ϊ����ռ���ȼ���
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
    ֻ����ռ���ȼ������ȼ�Ϊ0--15������ֵԽ�����ȼ�Խ��

    �ж����κ���:(�ٽ������ڴ˻����Ͻ����˿�Ƕ����չ)
    portDISABLE_INTERRUPTS();// ���ι��������ж�
    portENABLE_INTERRUPTS(); // ���ù��������ж�
    taskEXIT_CRITICAL(); // �˳��ٽ���
    taskENTER_CRITICAL(); // �����ٽ���           ..._FROM_ISR()Ϊ�жϼ�����ٽ�������
    vPortSetBASEPRI(x);      // ����BASEPRI�Ĵ�������������x(���ȼ��͵��ж�)��ֵ����ж� 
    ����0x50�������ȼ�����5���ж�(��4λΪ��Ӧ���ȼ���Ч)
  
   ���� Cortex-M3�ں�������8λ�Ĵ����������ж����ȼ�,����255�����ȼ�ʹ��,��һ����˵�ò�����ô��,
   ����STM32���豸����ͨ��NVIC����������,��8λ���򻮷�Ϊ��ռ�������ȼ�(��Ӧ)�������ȵȼ�
   ����freeRTOS�ֽ�����Ҫ��ռ���ȼ�(��Ӧ���ȼ����ù���),�ʻ��NVIC����"��4��������",
   ��8λ��ֻ�и�4λ��ռ���ȼ���Ч,��4λ��Ӧ���ȼ���Ч                                                        */

#define configPRIO_BITS             4                         // ϵͳ��ʹ�õ����ȼ�λ����STM32ʹ��4λ���ȼ�
/*  FreeRTOS���ܹ���� ���/��� ���ж����ȼ�
    !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY ��������Ϊ 0 !!!!  */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY (5<<configPRIO_BITS)  /* �൱�� 0x50���������ȼ� 5*/
#define configKERNEL_INTERRUPT_PRIORITY (15<<configPRIO_BITS)      /* ��Ϳɹ����ж����ȼ� */
/* !!!���Խ�FreeRTOS���������жϽ�������,��ֹ���,��������FreeRTOS������ж�����������!!! */

//�жϷ����� Ҳ�����޸���ʼ�ļ�
#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

// =====================
// �ڴ�Ͷ��������
// =====================
#define configSUPPORT_DYNAMIC_ALLOCATION 1                    // ֧�ֶ�̬�ڴ�����
#define configUSE_MALLOC_FAILED_HOOK 0                        // �����ڴ�����ʧ�ܹ��Ӻ���
#define configCHECK_FOR_STACK_OVERFLOW 1                      // ����ջ�����⹦��

// =====================
// �����ʱ���������
// =====================
#define configUSE_TIMERS           1                          // ���������ʱ��
#define configTIMER_TASK_PRIORITY  (configMAX_PRIORITIES-1)   // �����ʱ�����ȼ�
#define configTIMER_QUEUE_LENGTH   10                         // �����ʱ�����г���

// =====================
// ����ʱͳ������
// =====================
#define configGENERATE_RUN_TIME_STATS 0                      // ��������ʱ��ͳ�ƹ���
#define configUSE_STATS_FORMATTING_FUNCTIONS 1               // ����ͳ�Ƹ�ʽ������

// =====================
// ���Ժ�״̬��ȡ����
// =====================
/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_xTaskGetSchedulerState 1     // ������ȡ������״̬�Ĺ���
#define INCLUDE_eTaskGetState 1              // ������ȡ����״̬�Ĺ���
#define INCLUDE_xTimerPendFunctionCall 1     // ��������ʱ���������õĹ���
#define INCLUDE_vTaskPrioritySet          1  // ���������������ȼ��Ĺ���
#define INCLUDE_uxTaskPriorityGet         1  // ������ȡ�������ȼ��Ĺ���
#define INCLUDE_vTaskDelete               1  // ����ɾ������Ĺ���
#define INCLUDE_vTaskCleanUpResources     1  // ��������������Դ�Ĺ���
#define INCLUDE_vTaskSuspend              1  // ������������Ĺ���
#define INCLUDE_vTaskDelayUntil           1  // �����ӳ�����ֱ���ض�ʱ��Ĺ���
#define INCLUDE_vTaskDelay                1  // ���������ӳٵĹ���
#define INCLUDE_xTaskGetSchedulerState    1  // ������ȡ������״̬�Ĺ���
#define INCLUDE_eTaskGetState             1  // ������ȡ����״̬�Ĺ���
#define INCLUDE_xTimerPendFunctionCall    1  // ��������ʱ���������õĹ���
#define INCLUDE_uxTaskGetStackHighWaterMark 1  // ������ȡ��ǰ�������Ĺ���

#endif  /* FREERTOS_CONFIG_H */
