#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "PWM.h"

int main(void){
	uint32_t  H_num=0,T1=1000;
	
	char I=0,T=0;
	PWM_Init();
	OLED_Init();
	OLED_ShowString(1,1,"CCR_Num:");
	while(1){
		OLED_ShowNum(1,10,H_num,5);
		if(T>50)
		{
			if(H_num==10||H_num==0){I=!I;}
			if(I){
				H_num++;
			}
			else{
				H_num--;
			}
			PWM_Hight_Set(H_num);
			T1=1000;
			T=-1;
		}
		T++;
	}
	return 0;
}
