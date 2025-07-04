#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H
// =====================
// CPU和堆栈配置
// =====================
#define configCPU_CLOCK_HZ         ((unsigned long)72000000)  // CPU时钟频率设置为72MHz
#define configTICK_RATE_HZ         ((TickType_t)1000)         // 系统滴答频率设置为每秒1000次（1ms）
#define configMINIMAL_STACK_SIZE   ((unsigned short)128)      // 每个任务的最小堆栈大小设置为128字节
#define configTOTAL_HEAP_SIZE      ((size_t)(10*1024))       // 总堆内存大小设置为10KB
#define configMAX_TASK_NAME_LEN    (16)                       // 任务名称的最大长度设置为16个字符
#define configTIMER_TASK_STACK_DEPTH (configMINIMAL_STACK_SIZE*2) // 软件定时器任务堆栈大小
#define configUSE_IDLE_HOOK         0                         // 禁用空闲钩子函数，空闲钩子在系统空闲时调用
#define configUSE_TICK_HOOK         0                         // 禁用滴答钩子函数，滴答钩子在每个系统滴答定时器中调用

// =====================
// 任务和调度器相关配置
// =====================
#define configUSE_PREEMPTION        1                         // 启用抢占式调度，允许高优先级任务中断低优先级任务
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1             // 启用硬件计算下一任务(实际上是使用了硬件平台的特殊指令),较软件计算下一任务更高效
#define configMAX_PRIORITIES       (32)                       // 最大任务优先级数量设置为32
/*
  为什么是32位，因为"configUSE_PORT_OPTIMISED_TASK_SELECTION"启用了硬件计算下一任务的方式,
  而STM32支持这种方式,且因为是32位的原因,故最高仅支持32(0~31)个任务等级
*/
#define configUSE_TRACE_FACILITY    1                         // 启用跟踪功能，允许记录任务的运行时间等信息
#define configUSE_16_BIT_TICKS      0                         // 禁用16位滴答计数，使用32位滴答计数
#define configUSE_QUEUE_SETS        1                         // 启用队列集合
#define configUSE_TASK_NOTIFICATIONS 1                        // 开启任务通知功能
#define configUSE_MUTEXES           1                         // 使用互斥信号量
#define configUSE_RECURSIVE_MUTEXES 1                         // 使用递归互斥信号量
#define configUSE_COUNTING_SEMAPHORES 1                       // 启用计数信号量
#define configQUEUE_REGISTRY_SIZE   10                        // 设置可以注册的信号量和消息队列个数
#define configUSE_TIME_SLICING      1                         // 启用时间片调度，允许任务在时间片结束时进行切换
#define configIDLE_SHOULD_YIELD     1                         // 空闲任务应当让出CPU，允许其他任务运行

// =====================
// 中断优先级及其系统中断接口
// =====================

/*  在STM32使用由于中断优先级设置采用的是库函数，因此请必须保证优先级设置为可抢占优先级：
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
    只有抢占优先级，优先级为0--15级，数值越大优先级越低

    中断屏蔽函数:(临界区是在此基础上进行了可嵌套扩展)
    portDISABLE_INTERRUPTS();// 屏蔽管理区域中断
    portENABLE_INTERRUPTS(); // 启用管理区域中断
    taskEXIT_CRITICAL(); // 退出临界区
    taskENTER_CRITICAL(); // 进入临界区           ..._FROM_ISR()为中断级别的临界区代码
    vPortSetBASEPRI(x);      // 设置BASEPRI寄存器，用于屏蔽x(优先级低的中断)数值大的中断 
    例如0x50屏蔽优先级大于5的中断(低4位为响应优先级无效)
  
   这是 Cortex-M3内核设置了8位寄存器来配置中断优先级,故由255个优先级使用,但一般来说用不了这么多,
   故在STM32等设备中则通过NVIC来进行配置,将8位区域划分为抢占与子优先级(响应)两种优先等级
   而在freeRTOS种仅仅需要抢占优先级(响应优先级不好管理),故会对NVIC进行"第4方案分组",
   即8位中只有高4位抢占优先级有效,低4位响应优先级无效                                                        */

#define configPRIO_BITS             4                         // 系统中使用的优先级位数，STM32使用4位优先级
/*  FreeRTOS所能管理的 最高/最低 的中断优先级
    !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY 不能设置为 0 !!!!  */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY (5<<configPRIO_BITS)  /* 相当于 0x50，或者优先级 5*/
#define configKERNEL_INTERRUPT_PRIORITY (15<<configPRIO_BITS)      /* 最低可管理中断优先级 */
/* !!!可以将FreeRTOS管理的相关中断进行屏蔽,防止打断,而不属于FreeRTOS管理的中断能正常运行!!! */

//中断服务函数 也可以修改起始文件
#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

// =====================
// 内存和堆相关配置
// =====================
#define configSUPPORT_DYNAMIC_ALLOCATION 1                    // 支持动态内存申请
#define configUSE_MALLOC_FAILED_HOOK 0                        // 禁用内存申请失败钩子函数
#define configCHECK_FOR_STACK_OVERFLOW 1                      // 启用栈溢出检测功能

// =====================
// 软件定时器相关配置
// =====================
#define configUSE_TIMERS           1                          // 启用软件定时器
#define configTIMER_TASK_PRIORITY  (configMAX_PRIORITIES-1)   // 软件定时器优先级
#define configTIMER_QUEUE_LENGTH   10                         // 软件定时器队列长度

// =====================
// 运行时统计配置
// =====================
#define configGENERATE_RUN_TIME_STATS 0                      // 禁用运行时间统计功能
#define configUSE_STATS_FORMATTING_FUNCTIONS 1               // 启用统计格式化功能

// =====================
// 调试和状态获取配置
// =====================
/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_xTaskGetSchedulerState 1     // 包含获取调度器状态的功能
#define INCLUDE_eTaskGetState 1              // 包含获取任务状态的功能
#define INCLUDE_xTimerPendFunctionCall 1     // 包含挂起定时器函数调用的功能
#define INCLUDE_vTaskPrioritySet          1  // 包含设置任务优先级的功能
#define INCLUDE_uxTaskPriorityGet         1  // 包含获取任务优先级的功能
#define INCLUDE_vTaskDelete               1  // 包含删除任务的功能
#define INCLUDE_vTaskCleanUpResources     1  // 包含清理任务资源的功能
#define INCLUDE_vTaskSuspend              1  // 包含挂起任务的功能
#define INCLUDE_vTaskDelayUntil           1  // 包含延迟任务直到特定时间的功能
#define INCLUDE_vTaskDelay                1  // 包含任务延迟的功能
#define INCLUDE_xTaskGetSchedulerState    1  // 包含获取调度器状态的功能
#define INCLUDE_eTaskGetState             1  // 包含获取任务状态的功能
#define INCLUDE_xTimerPendFunctionCall    1  // 包含挂起定时器函数调用的功能
#define INCLUDE_uxTaskGetStackHighWaterMark 1  // 包含获取当前任务句柄的功能

#endif  /* FREERTOS_CONFIG_H */
