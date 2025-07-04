#include "LOG_SYSTEM/LOG.h"
#include "Pin_Config.h"
#include "sysctl.h"
#include "sleep.h"
#include "GUI\GUI_APP\MY_GUI.h"
#include "GUI\LCD_Button\Button.h"
#include "gpiohs.h"

void Hardware_Init(void){
    // LCD
    fpioa_set_function(PIN_LCD_CS,  FUNC_LCD_CS);
    fpioa_set_function(PIN_LCD_RST, FUNC_LCD_RST);
    fpioa_set_function(PIN_LCD_RS,  FUNC_LCD_RS);
    fpioa_set_function(PIN_LCD_WR,  FUNC_LCD_WR);
    // LCD需要1.8V供电
    sysctl_set_power_mode(SYSCTL_POWER_BANK6,SYSCTL_POWER_V18);
    /* 使能SPI0和DVP */
    sysctl_set_spi0_dvp_data(1);
    // 矩阵按键
    fpioa_set_function(PIN_KEYPAD1, FUNC_KEYPAD1);
    fpioa_set_function(PIN_KEYPAD2, FUNC_KEYPAD2);
    fpioa_set_function(PIN_KEYPAD3, FUNC_KEYPAD3);
    // 中断控制器初始化
    plic_init();
    // 开启系统中断
    sysctl_enable_irq();
}

int main(){
    Hardware_Init();
    GUI_Init();
    while(1){
        lv_task_handler();
        msleep(5);
    }
    return 0;
}
// cmake ../ -DPROJ=Main -G "MinGW Makefiles"
