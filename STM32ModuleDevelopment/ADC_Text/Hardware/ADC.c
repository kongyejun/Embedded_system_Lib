#include "ADC.h"
#define GPIO_Pinx GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7
#define SINGLE_LANES 10
#define CNT 4

uint16_t ADC_Four_ARR[CNT];
//定义存放ADC最终数据的数组
float ADC_AVGDATA[4]={0};

/*******************************************************************************************************************************************************
	*这里使用了连续转换模式来不间断的对ADC进行采样，不需要外部干涉，在对数据采样效率的上是比较好的！（具有较高的实时性）
	*
	*1.方案一：不过要是考虑在中断函数中不建议长时间占用CPU的话，就可以设置为 “单次转换+扫描模式” 的方式来手动进行触发转换ADC
	*	 				然后在DMA搬运完成每组10个数据后，产生中断，在中断中使用“滤波算法”，在将数据放到可被调用的数据组中。
	*         不过这会有一个缺点，就是ADC转换需要手动开启，在开启后还需要等待ADC转换一定量的数据，如果我们将采样时间设置的比较长的话，就会等待比较长的时间
	*
	*2.方案二：使用手动DMA转运的方式来获取ADC的值，这样以来也能解决在运行其他程序时，频繁被DMA触发中断导致程序混乱，
	* 				只需要在需要ADC数据的时候开启DMA，在搬运指定的数据量后，将标志某一状态位，这样不仅能解决方案一的在开启ADC转换时
	*         需要等待ADC转换结束的缺点，同时也能满足需求。不过其实并没有改善多少
	*
	*3.方案三：关闭DMA中断，让ADC与DMA一直工作，设置一个滤波函数API对数据进行处理,在处理时关闭DMA转运，结束时设置DMA传输计数器的值，再开启DMA转运
	*   			这样一来就能保证在较高实时性的同时还能不频繁产生中断，影响CPU------------------(实验发现这方法会导致DMA数据与ADC数据传输不对齐)
	*
	*最终决定采用方案三!!!!!!!!!!!!!!!!!
*********************************************************************************************************************************************************/

/**************************************************************************
函数功能：ADC的DAM外设初始化，用以弥补ADC数据覆盖的缺点-----------------------有BUG
入口参数：无
返回  值：无
描    述：用于快速运输ADC转换的数据，防止4路ADC的数据互相覆盖
					触发条件有：触发源，传输计数器不为0，DMA使能
					
弊		端：DMA传送的优点是以增加系统硬件的复杂性和成本为代价的，因为DMA是用硬件控制代替软件控制的。另外，DMA传送期间CPU被挂起，部分或完全失去对系统总线的控制，
					这可能会影响CPU对中断请求的及时响应与处理。因此，在一些小系统或速度要求不高、数据传输量不大的系统中，一般并不用DMA方式。
					因为DMA允许外设直接访问内存，从而形成对总线的独占。这在实时性强的硬实时系统嵌入式开发中将会造成中断延时过长。

**************************************************************************/
void ADC_DAM_Init()
{
	//开启DMA的RCC时钟
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	
	//DAM初始化
	DMA_InitTypeDef DMA_InitStruct;
	DMA_StructInit(&DMA_InitStruct);																		  		  //DMA通道初始化
	//DMA外设参数
	DMA_InitStruct.DMA_PeripheralBaseAddr =(uint32_t)(&(ADC1->DR));	  					//启动传输前装入实际RAM地址
	DMA_InitStruct.DMA_PeripheralDataSize =DMA_PeripheralDataSize_HalfWord;			//数据宽度为16位
	DMA_InitStruct.DMA_PeripheralInc =DMA_PeripheralInc_Disable;				  			//外设地址寄存器不变
	//DMA的寄存器参数
	DMA_InitStruct.DMA_MemoryBaseAddr =(uint32_t)&(ADC_Four_ARR[0]);           	//设置接收缓冲区首地址
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	    			//数据宽度为16位																						
	DMA_InitStruct.DMA_MemoryInc =DMA_MemoryInc_Enable;													//内存地址寄存器递增
	//DMA通道设置
	DMA_InitStruct.DMA_BufferSize = CNT;			  																//DMA通道的DMA缓存的大小,传输次数(传输计数器设定)
	DMA_InitStruct.DMA_DIR=DMA_DIR_PeripheralSRC;																//数据传输方向，从外设到内存
	DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;																//正常模式\自动重装模式
	DMA_InitStruct.DMA_Priority = DMA_Priority_Medium; 													//DMA通道x的优先级 
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;																		//不使用软件触发
	
	DMA_Init(DMA1_Channel1,&DMA_InitStruct);																		//注意此函数第一个参数是选择DMAx以及几号通道-----------且注意ADC1的DMA只在DMA1号上
	
	//启动DAM
	DMA_Cmd(DMA1_Channel1,ENABLE);
}
/**************************************************************************
函数功能：4路ADC初始化
入口参数：无
返回  值：ADC是否成功初始化
描    述：这里只初始化了ADC（4，5，6，7）！！
**************************************************************************/
FunctionalState ADC_Four_Init()
{
	//开启对应时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1|RCC_APB2Periph_GPIOA,ENABLE);
	//初始化GPIO口
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AIN;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pinx;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);

	//ADC初始化
	ADC_InitTypeDef ADC_InitSruct;
	ADC_InitSruct.ADC_DataAlign=ADC_DataAlign_Right;													//转换结果右对齐
	ADC_InitSruct.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;							//不使用外部触发转换，采用软件触发
	ADC_InitSruct.ADC_Mode=ADC_Mode_Independent;															//只使用一个ADC，独立模式
	ADC_InitSruct.ADC_NbrOfChannel=CNT;															  				//几个转换通道(只用扫描模式时才有用)
	ADC_InitSruct.ADC_ScanConvMode=ENABLE;																		//使用扫描模式，多通道时使用
	ADC_InitSruct.ADC_ContinuousConvMode=ENABLE;															//禁止连续转换模式
	ADC_Init(ADC1,&ADC_InitSruct);
		
	/* 配置ADC时钟为8分频，即9M */
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);
	//配置ADC通道
	ADC_RegularChannelConfig(ADC1,ADC_Channel_4,1,ADC_SampleTime_71Cycles5);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_5,2,ADC_SampleTime_71Cycles5);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_6,3,ADC_SampleTime_71Cycles5);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_7,4,ADC_SampleTime_71Cycles5);

//	//ADC中断使能配置
//	ADC_ITConfig(ADC1,ADC_IT_EOC,DISABLE);																	//禁止ADC中断
//	//NVIC配置
//	NVIC_InitTypeDef NVIC_InitStruct;
//	NVIC_InitStruct.NVIC_IRQChannel=ADC1_2_IRQn;														//配置ADC1_2的通道
//	NVIC_InitStruct.NVIC_IRQChannelCmd=DISABLE;															//禁止ADC进入NVIC中断
//	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=0;
//	NVIC_InitStruct.NVIC_IRQChannelSubPriority=0;
//	NVIC_Init(&NVIC_InitStruct);
	
	//使能ADC的DMA请求
	ADC_DMACmd(ADC1,ENABLE);
	
	ADC_DAM_Init();																														//配置DMA模式

	//开启ADC
	ADC_Cmd(ADC1,ENABLE);
	
	/* 重置ADC校准 */
	ADC_ResetCalibration(ADC1);
	/* 等待初始化完成 */
	while(ADC_GetResetCalibrationStatus(ADC1))
	/* 开始校准 */
	ADC_StartCalibration(ADC1);
	/* 等待校准完成 */
	while (ADC_GetCalibrationStatus(ADC1));
	
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);                                    //ADC转换触发
	return ADC_CR2_SWSTART==1?ENABLE:DISABLE;																	//如果软件触发模式打开，代表ADC已经成功初始化
}
/**************************************************************************
函数功能：均值滤波算法
入口参数：无
返回  值：无
**************************************************************************/
void Mean_Filtering()
{
	//定义变量
	char i,j; uint16_t Max[4],Min[4];
	uint32_t CONT[4]={0};
	
	Max[0]=Max[1]=Max[2]=Max[3]=0;Min[0]=Min[1]=Min[2]=Min[3]=4096;
	for(i=0;i<SINGLE_LANES;i++,j=0)
	{
		while(j<CNT){
			if(Min[j]>ADC_Four_ARR[j]){
				Min[j]=ADC_Four_ARR[j];
			} 
			if(Max[j]<ADC_Four_ARR[j]){
				Max[j]=ADC_Four_ARR[j];
			}
			CONT[j]+=ADC_Four_ARR[j];
			j++;
		}
	}
	for(i=0;i<CNT;i++){
		ADC_AVGDATA[i]=((CONT[i]-Max[i]-Min[i])*1.0)/(SINGLE_LANES-2);
	}
	
}
////--------------------------------------------------------------------------------------------------------------------------方案一
///************************************************************************************
//函数功能：ADC手动开启转换------------------------------------------记得将ADC的模式改为 单次转换，并且设置位软件触发转换模式,DMA是自动重装模式
//入口参数：无
//返回  值：12位ADC通道数据
//*************************************************************************************/
//void ADC_GetOneDATA()
//{
//	//用软件触发ADC转换
//	ADC_SoftwareStartConvCmd(ADC1,ENABLE);
//	//等待ADC转换完成
//	while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC)!=SET);
//}
//void ADC_Start()
//{
//	uint32_t CON;
//	char i,j;
//	uint16_t Max,Min;
//	//收集10次数据
//	for(i=0;i<SINGLE_LANES;i++){
//		ADC_GetOneDATA();
//	}
//	//采用中值滤波
//	Mean_Filtering();
//}

////---------------------------------------------------------------------------------------------------------------------------方案二
///**************************************************************************
//函数功能：ADC的DMA开启函数---------------记得开启中断通道，以及在初始化时不要使能DMA
//入口参数：无
//返回  值：无
//**************************************************************************/
//void ADC_DMA_Start()
//{
//	DMA_Cmd(DMA1_Channel1,ENABLE);
//	while(DMA_GetFlagStatus(DMA1_FLAG_TC1)!=SET);//等待转换完成
//	DMA_Cmd(DMA1_Channel1,DISABLE);
//	DMA_SetCurrDataCounter(DMA1_Channel1,CNT*SINGLE_LANES);//设置传输计数器的值，方便下一次使用
//}

///**************************************************************************
//函数功能：ADC的DMA中断函数版滤波算法
//入口参数：无
//返回  值：无
//**************************************************************************/
//void DMA1_Channel1_IRQHandler()
//{
//	//变量定义
//	uint32_t CON;
//	char i,j;
//	uint16_t Max,Min;
//	
//	ADC_AVGDATA[4]=0;//设为状态位，表示现在正在转换，不可读取
//	
//	//滤波算法
//	DMA_Cmd(DMA1_Channel1,DISABLE);//关闭DMA搬运数据，避免对排序造成影响
//	//采用中值滤波
//	Mean_Filtering();
//	ADC_AVGDATA[4]=1;//可以读取
//	
//	//结束转换
//	DMA_SetCurrDataCounter(DMA1_Channel1,CNT*SINGLE_LANES);
//	DMA_Cmd(DMA1_Channel1,DISABLE);
//	DMA_ClearFlag(DMA1_FLAG_TC1);
//}
//-------------------------------------------------------------------------------------------------------------方案三
/**************************************************************************
函数功能：ADC_手动滤波算法---------------可能出现在第一次之前，没有调用调用该算法时，数据数组是无效值
入口参数：无
返回  值：无
**************************************************************************/
void ADC_Filtering_Strat()
{
	int i;
	//赛道信息-----用以归一化处理
	static uint16_t L1[2]={4096,0};
	static uint16_t L2[2]={4096,0};
	static uint16_t R2[2]={4096,0};
	static uint16_t R1[2]={4096,0};
		
	//采用均值滤波
	Mean_Filtering();
	//对数据进行归一化处理------------------增强赛车对不同环境下的适用性
	
	//
}


