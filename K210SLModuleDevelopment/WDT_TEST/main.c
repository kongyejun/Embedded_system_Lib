#include "LOG_SYSTEM/LOG.h"
#include "uarths.h"
#include "wdt.h"
#include "sleep.h"

#define WDT_TIME_OUT 2000

/**
 * @brief  硬件初始化
 * @param  无
 * @retval None
 */
void Hardware_Init(void){
    return ;
}

int WDT0_Callback(){
    EMLOG(LOG_ERROR,"WDT Time OUT, But don't set system restart\n");
    wdt_clear_interrupt(WDT_DEVICE_0);
    return 0;
}

int main(){
    uarths_init();
    //打印系统初次运行信息
    EMLOG(LOG_INFO,"System Init Success!!!\n");
    //初始化看门狗
    wdt_init(WDT_DEVICE_0,WDT_TIME_OUT,WDT0_Callback,NULL);
    //主程序
    int WDT_CONT=0;
    while(1){
        msleep(1000);
        if(WDT_CONT++ < 5){
            EMLOG(LOG_INFO,"WDT Feer Time%d\n",WDT_CONT);
            wdt_feed(WDT_DEVICE_0);
        }
    }

    return 0;
}