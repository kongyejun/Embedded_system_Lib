#include "pin_config.h"
#include "LCD_LIB/lcd.h"
#include "sysctl.h"
#include "stdio.h"
#include "sleep.h"
#include "FT6236.h"
#include "printf.h"
volatile int Point_IS = 0;
//定义初始化函数
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

int main(){
    char buff[30]="Point1: (0,0)";
    uint32_t color = 0xFFFFFFFF;
    Hardware_Init();
    /* 系统中断初始化，并使能全局中断 */
    plic_init();
    sysctl_enable_irq();
    lcd_init();
    lcd_clear((uint16_t)0xffff);
    lcd_clear(WHITE);
    lcd_draw_string(20,20,"Hello Master!!",BLUE);
    lcd_draw_string(20,50,buff,BLACK);
    FT6236_Init();
    while(1){
        if(Point_IS){
            Point_IS=0;
            FT6236_GetPXY();
            if(FT_Point1.state & TP_PRES_DOWN){
                FT_Point1.state &= ~TP_PRES_DOWN;//使程序在接触点有变化使改变坐标
                lcd_fill_RATE(80,50,20,80,&color,10*80);
                sprintf(buff,"Point1: (%d,%d)",FT_Point1.x,FT_Point1.y);
                lcd_draw_string(20,50,buff,BLACK);
            }
            if(FT_Point2.state & TP_PRES_DOWN){
                FT_Point2.state &= ~TP_PRES_DOWN;
                lcd_fill_RATE(80,80,20,80,&color,10*80);
                sprintf(buff,"Point2: (%d,%d)",FT_Point2.x,FT_Point2.y);
                lcd_draw_string(20,80,buff,BLACK);
            }
        }
        msleep(80);
    }
    return 0;
}