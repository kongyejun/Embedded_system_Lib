#include "ADC.h"
#define GPIO_Pinx GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7
#define SINGLE_LANES 10
#define CNT 4

uint16_t ADC_Four_ARR[CNT];
//������ADC�������ݵ�����
float ADC_AVGDATA[4]={0};

/*******************************************************************************************************************************************************
	*����ʹ��������ת��ģʽ������ϵĶ�ADC���в���������Ҫ�ⲿ���棬�ڶ����ݲ���Ч�ʵ����ǱȽϺõģ������нϸߵ�ʵʱ�ԣ�
	*
	*1.����һ������Ҫ�ǿ������жϺ����в����鳤ʱ��ռ��CPU�Ļ����Ϳ�������Ϊ ������ת��+ɨ��ģʽ�� �ķ�ʽ���ֶ����д���ת��ADC
	*	 				Ȼ����DMA�������ÿ��10�����ݺ󣬲����жϣ����ж���ʹ�á��˲��㷨�����ڽ����ݷŵ��ɱ����õ��������С�
	*         ���������һ��ȱ�㣬����ADCת����Ҫ�ֶ��������ڿ�������Ҫ�ȴ�ADCת��һ���������ݣ�������ǽ�����ʱ�����õıȽϳ��Ļ����ͻ�ȴ��Ƚϳ���ʱ��
	*
	*2.��������ʹ���ֶ�DMAת�˵ķ�ʽ����ȡADC��ֵ����������Ҳ�ܽ����������������ʱ��Ƶ����DMA�����жϵ��³�����ң�
	* 				ֻ��Ҫ����ҪADC���ݵ�ʱ����DMA���ڰ���ָ�����������󣬽���־ĳһ״̬λ�����������ܽ������һ���ڿ���ADCת��ʱ
	*         ��Ҫ�ȴ�ADCת��������ȱ�㣬ͬʱҲ���������󡣲�����ʵ��û�и��ƶ���
	*
	*3.���������ر�DMA�жϣ���ADC��DMAһֱ����������һ���˲�����API�����ݽ��д���,�ڴ���ʱ�ر�DMAת�ˣ�����ʱ����DMA�����������ֵ���ٿ���DMAת��
	*   			����һ�����ܱ�֤�ڽϸ�ʵʱ�Ե�ͬʱ���ܲ�Ƶ�������жϣ�Ӱ��CPU------------------(ʵ�鷢���ⷽ���ᵼ��DMA������ADC���ݴ��䲻����)
	*
	*���վ������÷�����!!!!!!!!!!!!!!!!!
*********************************************************************************************************************************************************/

/**************************************************************************
�������ܣ�ADC��DAM�����ʼ���������ֲ�ADC���ݸ��ǵ�ȱ��-----------------------��BUG
��ڲ�������
����  ֵ����
��    �������ڿ�������ADCת�������ݣ���ֹ4·ADC�����ݻ��า��
					���������У�����Դ�������������Ϊ0��DMAʹ��
					
��		�ˣ�DMA���͵��ŵ���������ϵͳӲ���ĸ����Ժͳɱ�Ϊ���۵ģ���ΪDMA����Ӳ�����ƴ���������Ƶġ����⣬DMA�����ڼ�CPU�����𣬲��ֻ���ȫʧȥ��ϵͳ���ߵĿ��ƣ�
					����ܻ�Ӱ��CPU���ж�����ļ�ʱ��Ӧ�봦����ˣ���һЩСϵͳ���ٶ�Ҫ�󲻸ߡ����ݴ����������ϵͳ�У�һ�㲢����DMA��ʽ��
					��ΪDMA��������ֱ�ӷ����ڴ棬�Ӷ��γɶ����ߵĶ�ռ������ʵʱ��ǿ��ӲʵʱϵͳǶ��ʽ�����н�������ж���ʱ������

**************************************************************************/
void ADC_DAM_Init()
{
	//����DMA��RCCʱ��
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	
	//DAM��ʼ��
	DMA_InitTypeDef DMA_InitStruct;
	DMA_StructInit(&DMA_InitStruct);																		  		  //DMAͨ����ʼ��
	//DMA�������
	DMA_InitStruct.DMA_PeripheralBaseAddr =(uint32_t)(&(ADC1->DR));	  					//��������ǰװ��ʵ��RAM��ַ
	DMA_InitStruct.DMA_PeripheralDataSize =DMA_PeripheralDataSize_HalfWord;			//���ݿ��Ϊ16λ
	DMA_InitStruct.DMA_PeripheralInc =DMA_PeripheralInc_Disable;				  			//�����ַ�Ĵ�������
	//DMA�ļĴ�������
	DMA_InitStruct.DMA_MemoryBaseAddr =(uint32_t)&(ADC_Four_ARR[0]);           	//���ý��ջ������׵�ַ
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	    			//���ݿ��Ϊ16λ																						
	DMA_InitStruct.DMA_MemoryInc =DMA_MemoryInc_Enable;													//�ڴ��ַ�Ĵ�������
	//DMAͨ������
	DMA_InitStruct.DMA_BufferSize = CNT;			  																//DMAͨ����DMA����Ĵ�С,�������(����������趨)
	DMA_InitStruct.DMA_DIR=DMA_DIR_PeripheralSRC;																//���ݴ��䷽�򣬴����赽�ڴ�
	DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;																//����ģʽ\�Զ���װģʽ
	DMA_InitStruct.DMA_Priority = DMA_Priority_Medium; 													//DMAͨ��x�����ȼ� 
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;																		//��ʹ���������
	
	DMA_Init(DMA1_Channel1,&DMA_InitStruct);																		//ע��˺�����һ��������ѡ��DMAx�Լ�����ͨ��-----------��ע��ADC1��DMAֻ��DMA1����
	
	//����DAM
	DMA_Cmd(DMA1_Channel1,ENABLE);
}
/**************************************************************************
�������ܣ�4·ADC��ʼ��
��ڲ�������
����  ֵ��ADC�Ƿ�ɹ���ʼ��
��    ��������ֻ��ʼ����ADC��4��5��6��7������
**************************************************************************/
FunctionalState ADC_Four_Init()
{
	//������Ӧʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1|RCC_APB2Periph_GPIOA,ENABLE);
	//��ʼ��GPIO��
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AIN;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pinx;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);

	//ADC��ʼ��
	ADC_InitTypeDef ADC_InitSruct;
	ADC_InitSruct.ADC_DataAlign=ADC_DataAlign_Right;													//ת������Ҷ���
	ADC_InitSruct.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;							//��ʹ���ⲿ����ת���������������
	ADC_InitSruct.ADC_Mode=ADC_Mode_Independent;															//ֻʹ��һ��ADC������ģʽ
	ADC_InitSruct.ADC_NbrOfChannel=CNT;															  				//����ת��ͨ��(ֻ��ɨ��ģʽʱ������)
	ADC_InitSruct.ADC_ScanConvMode=ENABLE;																		//ʹ��ɨ��ģʽ����ͨ��ʱʹ��
	ADC_InitSruct.ADC_ContinuousConvMode=ENABLE;															//��ֹ����ת��ģʽ
	ADC_Init(ADC1,&ADC_InitSruct);
		
	/* ����ADCʱ��Ϊ8��Ƶ����9M */
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);
	//����ADCͨ��
	ADC_RegularChannelConfig(ADC1,ADC_Channel_4,1,ADC_SampleTime_71Cycles5);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_5,2,ADC_SampleTime_71Cycles5);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_6,3,ADC_SampleTime_71Cycles5);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_7,4,ADC_SampleTime_71Cycles5);

//	//ADC�ж�ʹ������
//	ADC_ITConfig(ADC1,ADC_IT_EOC,DISABLE);																	//��ֹADC�ж�
//	//NVIC����
//	NVIC_InitTypeDef NVIC_InitStruct;
//	NVIC_InitStruct.NVIC_IRQChannel=ADC1_2_IRQn;														//����ADC1_2��ͨ��
//	NVIC_InitStruct.NVIC_IRQChannelCmd=DISABLE;															//��ֹADC����NVIC�ж�
//	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=0;
//	NVIC_InitStruct.NVIC_IRQChannelSubPriority=0;
//	NVIC_Init(&NVIC_InitStruct);
	
	//ʹ��ADC��DMA����
	ADC_DMACmd(ADC1,ENABLE);
	
	ADC_DAM_Init();																														//����DMAģʽ

	//����ADC
	ADC_Cmd(ADC1,ENABLE);
	
	/* ����ADCУ׼ */
	ADC_ResetCalibration(ADC1);
	/* �ȴ���ʼ����� */
	while(ADC_GetResetCalibrationStatus(ADC1))
	/* ��ʼУ׼ */
	ADC_StartCalibration(ADC1);
	/* �ȴ�У׼��� */
	while (ADC_GetCalibrationStatus(ADC1));
	
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);                                    //ADCת������
	return ADC_CR2_SWSTART==1?ENABLE:DISABLE;																	//����������ģʽ�򿪣�����ADC�Ѿ��ɹ���ʼ��
}
/**************************************************************************
�������ܣ���ֵ�˲��㷨
��ڲ�������
����  ֵ����
**************************************************************************/
void Mean_Filtering()
{
	//�������
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
////--------------------------------------------------------------------------------------------------------------------------����һ
///************************************************************************************
//�������ܣ�ADC�ֶ�����ת��------------------------------------------�ǵý�ADC��ģʽ��Ϊ ����ת������������λ�������ת��ģʽ,DMA���Զ���װģʽ
//��ڲ�������
//����  ֵ��12λADCͨ������
//*************************************************************************************/
//void ADC_GetOneDATA()
//{
//	//���������ADCת��
//	ADC_SoftwareStartConvCmd(ADC1,ENABLE);
//	//�ȴ�ADCת�����
//	while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC)!=SET);
//}
//void ADC_Start()
//{
//	uint32_t CON;
//	char i,j;
//	uint16_t Max,Min;
//	//�ռ�10������
//	for(i=0;i<SINGLE_LANES;i++){
//		ADC_GetOneDATA();
//	}
//	//������ֵ�˲�
//	Mean_Filtering();
//}

////---------------------------------------------------------------------------------------------------------------------------������
///**************************************************************************
//�������ܣ�ADC��DMA��������---------------�ǵÿ����ж�ͨ�����Լ��ڳ�ʼ��ʱ��Ҫʹ��DMA
//��ڲ�������
//����  ֵ����
//**************************************************************************/
//void ADC_DMA_Start()
//{
//	DMA_Cmd(DMA1_Channel1,ENABLE);
//	while(DMA_GetFlagStatus(DMA1_FLAG_TC1)!=SET);//�ȴ�ת�����
//	DMA_Cmd(DMA1_Channel1,DISABLE);
//	DMA_SetCurrDataCounter(DMA1_Channel1,CNT*SINGLE_LANES);//���ô����������ֵ��������һ��ʹ��
//}

///**************************************************************************
//�������ܣ�ADC��DMA�жϺ������˲��㷨
//��ڲ�������
//����  ֵ����
//**************************************************************************/
//void DMA1_Channel1_IRQHandler()
//{
//	//��������
//	uint32_t CON;
//	char i,j;
//	uint16_t Max,Min;
//	
//	ADC_AVGDATA[4]=0;//��Ϊ״̬λ����ʾ��������ת�������ɶ�ȡ
//	
//	//�˲��㷨
//	DMA_Cmd(DMA1_Channel1,DISABLE);//�ر�DMA�������ݣ�������������Ӱ��
//	//������ֵ�˲�
//	Mean_Filtering();
//	ADC_AVGDATA[4]=1;//���Զ�ȡ
//	
//	//����ת��
//	DMA_SetCurrDataCounter(DMA1_Channel1,CNT*SINGLE_LANES);
//	DMA_Cmd(DMA1_Channel1,DISABLE);
//	DMA_ClearFlag(DMA1_FLAG_TC1);
//}
//-------------------------------------------------------------------------------------------------------------������
/**************************************************************************
�������ܣ�ADC_�ֶ��˲��㷨---------------���ܳ����ڵ�һ��֮ǰ��û�е��õ��ø��㷨ʱ��������������Чֵ
��ڲ�������
����  ֵ����
**************************************************************************/
void ADC_Filtering_Strat()
{
	int i;
	//������Ϣ-----���Թ�һ������
	static uint16_t L1[2]={4096,0};
	static uint16_t L2[2]={4096,0};
	static uint16_t R2[2]={4096,0};
	static uint16_t R1[2]={4096,0};
		
	//���þ�ֵ�˲�
	Mean_Filtering();
	//�����ݽ��й�һ������------------------��ǿ�����Բ�ͬ�����µ�������
	
	//
}


