#include "GUI\LVGL\lvgl.h"
#include "Pin_Config.h"
#include "fpioa.h"
#include "sysctl.h"
#include "GUI\LVGL\porting\lv_port_disp.h"
#include "GUI\LVGL\porting\lv_port_indev.h"
#include "TIME_ctl.h"
#include "sleep.h"
#include "GUI\LCD_SHOW\lcd.h"
#include "GUI\GUI_APP\MY_GUI.h"
#include "ICM2060\ICM2060.h"

void Hardware_Init(void){
    //LCD
    fpioa_set_function(PIN_LCD_CS,  FUNC_LCD_CS);
    fpioa_set_function(PIN_LCD_RST, FUNC_LCD_RST);
    fpioa_set_function(PIN_LCD_RS,  FUNC_LCD_RS);
    fpioa_set_function(PIN_LCD_WR,  FUNC_LCD_WR);
    //LCD需要1.8V供电
    sysctl_set_power_mode(SYSCTL_POWER_BANK6,SYSCTL_POWER_V18);
    /* 使能SPI0和DVP */
    sysctl_set_spi0_dvp_data(1);
    //触摸屏
    fpioa_set_function(PIN_FT_INT,  FUNC_FT_INT);
    fpioa_set_function(PIN_FT_SCL, FUNC_FT_SCL);
    fpioa_set_function(PIN_FT_SDA,  FUNC_FT_SDA);
}

void lvgl_bsp_init(){
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
    LVGL_tick_Init();
}

int main(void){
    Hardware_Init();
    lvgl_bsp_init();
    ICM2060_FULL_Init(); //初始化IMU
    My_GUI();
    float x=10.5,y=0.0005,z=0;
    while(1){
        lvgl_Horizontal_plates();
        lv_task_handler();
        Printf_ICM2060_EndData(&x,&y,&z);
        msleep(5);
    }

    return 0;
}