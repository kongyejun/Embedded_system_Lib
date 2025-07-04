#include "stm32f10x.h"

/*
void EXTI_DeInit(void);----恢复出厂设置
void EXTI_Init(EXTI_InitTypeDef* EXTI_InitStruct);-----初始化EXTI
void EXTI_StructInit(EXTI_InitTypeDef* EXTI_InitStruct);------获取EXTI的结构体参数
void EXTI_GenerateSWInterrupt(uint32_t EXTI_Line);------软件模拟中断
FlagStatus EXTI_GetFlagStatus(uint32_t EXTI_Line);-------查看指定的标志位是否被至1了（可以查看不被允许中断的标志位）
void EXTI_ClearFlag(uint32_t EXTI_Line);------清除指定的标志位
ITStatus EXTI_GetITStatus(uint32_t EXTI_Line);-------查看指定的标志位是否被置1了（查看可以中断的标志位）
void EXTI_ClearITPendingBit(uint32_t EXTI_Line);------清除指定的标志位
*/

uint16_t count_sensor;

//初始化红外计数器
void CountSensor_init(){
	count_sensor=0;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//开启GPIO外设的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//开启AFIO的时钟
	//GPIO初始化
	GPIO_InitTypeDef GPIO_initstruct;
	GPIO_initstruct.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_initstruct.GPIO_Pin=GPIO_Pin_8;
	GPIO_initstruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_initstruct);
	//AFIO通道选择
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource8);
	//EXTI配置
	EXTI_InitTypeDef EXTI_Initstruct;
	EXTI_Initstruct.EXTI_Line=EXTI_Line8;
	EXTI_Initstruct.EXTI_LineCmd=ENABLE;
	EXTI_Initstruct.EXTI_Mode=EXTI_Mode_Interrupt;
	EXTI_Initstruct.EXTI_Trigger=EXTI_Trigger_Falling;
	//初始化EXTI
	EXTI_Init(&EXTI_Initstruct);
	//设置NVIC寄存器
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//------------设置分组
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel=EXTI9_5_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=1;
	//初始化NVIC
	NVIC_Init(&NVIC_InitStruct);
}

uint16_t GetCountSensor()
{
	return count_sensor;
}

//中断函数
void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line8)==SET){//判断是否为8号信号通道引起的中断
		EXTI_ClearITPendingBit(EXTI_Line8);//清除中断标志位
		count_sensor++;
		EXTI_GenerateSWInterrupt(EXTI_Line9);
	}
}
