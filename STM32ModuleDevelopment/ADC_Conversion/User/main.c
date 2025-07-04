#include "stm32f10x.h"                  
#include "OLED.h"
#include "ADC.h"
#include "Delay.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
int main(void){
	uint16_t Value;
//	uint8_t ADC_Channe;
	OLED_Init();
	MY_ADC_Init();
	OLED_ShowString(1,1,"Value:");
	while(1)
	{
		Value=ADC_GetData();
		OLED_ShowNum(1,7,Value,5);
		
		Delay_ms(100);
	}
//	return 0;
}
