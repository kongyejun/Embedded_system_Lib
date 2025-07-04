#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "ADC.h"
extern float ADC_AVGDATA[4]; 
int main(void){
	ADC_Four_Init();
	OLED_Init();
	while(1){
		ADC_Filtering_Strat();
		OLED_ShowNum(1,1,(uint32_t)ADC_AVGDATA[0],5);
		OLED_ShowChar(1,6,'.');
		OLED_ShowNum(1,7,(uint32_t)(ADC_AVGDATA[0]-(uint32_t)ADC_AVGDATA[0])*100,2);
		OLED_ShowNum(2,1,(uint32_t)ADC_AVGDATA[1],5);
		OLED_ShowChar(2,6,'.');
		OLED_ShowNum(2,7,(uint32_t)(ADC_AVGDATA[1]-(uint32_t)ADC_AVGDATA[1])*100,2);
		OLED_ShowNum(3,1,(uint32_t)ADC_AVGDATA[2],5);
		OLED_ShowChar(3,6,'.');
		OLED_ShowNum(3,7,(uint32_t)(ADC_AVGDATA[2]-(uint32_t)ADC_AVGDATA[2])*100,2);
		Delay_ms(100);
	}
	return 0;
}
