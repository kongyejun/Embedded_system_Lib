#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"

void OLED_INFO(char* info,uint8_t char_nums){
	static int8_t new_row = 0,i = 0;
	OLED_ShowString(new_row,0,info);
	new_row += (uint8_t)(char_nums/16);// 处理字符串过长导致占用其他行的问题
	// 补齐空间
	for(i=char_nums%17;i<=15;i+=1)
		OLED_ShowChar(new_row,i,' ');
	new_row = OLED_Scroll_row(1,new_row);// 滚动显示,起始行始终处于有效信息的下一行
}
// 使编译器不优化这段代码，保留原样
#pragma GCC optimize ("O0")
int main(void){
	OLED_Init();
	OLED_INFO("Hello World!.......",19);
	Delay_ms(1000);
	OLED_INFO("Hello!",6);
	Delay_ms(1000);
	OLED_INFO("world!",6);
	Delay_ms(1000);
	OLED_INFO("HOW ARE YOU!",12);
	Delay_ms(1000);
	OLED_INFO("I AM GOOD!",10);
	while(1);
	return 0;
}
