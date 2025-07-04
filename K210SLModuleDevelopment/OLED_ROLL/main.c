#include "LOG_SYSTEM/LOG.h"
#include "Pin_Config.h"
#include "OLED.h"
#include "uarths.h"
#include "sleep.h"
/**
 * @brief  硬件初始化
 * @param  无
 * @retval None
 */
void Hardware_Init(void){
    fpioa_init();
    //软硬件绑定
    fpioa_set_function(PIN_OLED_SDA, FUNC_OLED_SDA);
    fpioa_set_function(PIN_OLED_SCL, FUNC_OLED_SCL);
}

void OLED_INFO(char* info,uint8_t char_nums){
	static int8_t new_row = 1;
	OLED_ShowString(new_row,1,info);
	new_row += ((uint8_t)(char_nums/16))*16;// 处理字符串过长导致占用其他行的问题
	new_row = OLED_Scroll_row(1,16,new_row);// 滚动显示,起始行始终处于有效信息的下一行
	OLED_ShowNum(new_row,2+char_nums,new_row,2);
}

#pragma GCC optimize ("O0")
int main(){
    int8_t i = 0;
    Hardware_Init();
    uarths_init();
    EMLOG(LOG_INFO,"next to OLED_Init\n");
    OLED_Init();
    EMLOG(LOG_INFO,"weclome to OLED_ROLL!\n");
    OLED_ShowString(7,1,"123");
	OLED_INFO("Hello World!",12);
	msleep(1000);
	OLED_INFO("Hello!",6);
    while(1){
        ;
    }
    return 0;
}
