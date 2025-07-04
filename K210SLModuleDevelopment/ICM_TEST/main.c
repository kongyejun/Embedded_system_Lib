#include "uarths.h"
#include "LOG_SYSTEM/LOG.h"
#include "Pin_Config.h"
#include "ICM2060.h"
#include "sleep.h"
#include "sysctl.h"
#include "LCD_SHOW\lcd.h"
#include "KEYPAD\KeyPad_bsp.h"
#include "I2C_ICM\I2C0_CTL.h"

#define LCD_VS_TFA 0
#define LCD_VS_VSA 200
#define LCD_VS_BFA 120
#define LCD_VS_VSP LCD_VS_VSA

// float Kp=10.2,Ki=0.0005,Kd = 0;   // PID参数----误差角直接补偿角速度
float Kp=1.4,Ki=0.0005,Kd = 0;   // PID参数----误差角转角速度单位补偿陀螺仪角速度
#define Kp_Step 0.1
#define Ki_Step 0.001
#define Kd_Step 0.05

static char num[40];

/**
 * @brief  硬件初始化
 * @param  无
 * @retval None
 */
void Hardware_Init(void){
    //ICM
    fpioa_set_function(PIN_ICM_SCLK,FUNC_ICM_SCLK);
    fpioa_set_function(PIN_ICM_SDA,FUNC_ICM_SDA);
    fpioa_set_function(PIN_ICM_INT,FUNC_ICM_INT);
    //LCD
    fpioa_set_function(PIN_LCD_CS,  FUNC_LCD_CS);
    fpioa_set_function(PIN_LCD_RST, FUNC_LCD_RST);
    fpioa_set_function(PIN_LCD_RS,  FUNC_LCD_RS);
    fpioa_set_function(PIN_LCD_WR,  FUNC_LCD_WR);
    /* 使能SPI0和DVP数据 */
    sysctl_set_spi0_dvp_data(1); 
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    //KeyPad
    fpioa_set_function(KEYPAD_LIFT  , FUNC_KEYPAD_L);
    fpioa_set_function(KEYPAD_MID   , FUNC_KEYPAD_M);
    fpioa_set_function(KEYPAD_RTGHT , FUNC_KEYPAD_R);
    //使能中断相关设置
    plic_init();
    sysctl_enable_irq();
}

// 在绘制LCD屏上绘制 横滚角 与 俯仰角 曲线
void lcd_draw_ICMcurve(){
    static uint16_t i=0,vsp=0;
    static uint8_t FLAG = 0;//屏幕滚动刷新标记
    ICM2060_PosSlove();
    Printf_ICM2060_EndData(&Kp,&Ki,&Kd);
    lcd_draw_point(i,-ICM_EndData.pitch+LCD_Y_MAX/2,RED);//居中显示
    lcd_draw_point(i,-ICM_EndData.roll+LCD_Y_MAX/2,YELLOW);
    i+=1;
    if(i>(LCD_VS_VSP-1)){
        FLAG|=0x01;i=0;
    }
    if(FLAG){
        vsp = vsp+1<LCD_VS_VSP?vsp+1:0;
        lcd_draw_LinePoint((vsp==0?LCD_VS_VSP-1:vsp-1),BLACK);
        lcd_VerticalScrollAdd(&vsp);
    }
}

void keyevnet_dispose(){
    static keypad_event ev;
    static int8_t FLAG_KiKpKd=0;
    if(Get_FIFO_Free()!=0){
        KeyFIFO_read(&ev);
        switch(ev){
            // 向左按键事件
            case KEY_LEFT_DOWN: EMLOG(LOG_INFO,"KEY_LEFT_DOWN\n");
                                if(!FLAG_KiKpKd)Kp+=Kp_Step;
                                else if(FLAG_KiKpKd==1)Ki+=Ki_Step;
                                else Kd+=Kd_Step;
                                break;
            // 向右按键事件
            case KEY_RIGHT_DOWN:EMLOG(LOG_INFO,"KEY_RIGHT_DOWN\n");
                                if(!FLAG_KiKpKd)Kp-=Kp_Step;
                                else if(FLAG_KiKpKd==1)Ki-=Ki_Step;
                                else Kd-=Kd_Step;
                                break;
            // 中间按键事件
            case KEY_MIDDLE_DOWN:EMLOG(LOG_INFO,"KEY_MIDDLE_DOWN\n");
                                FLAG_KiKpKd+=1;
                                FLAG_KiKpKd%=3;
                                break;
            default:break;
        }
        sprintf(num,"State:%s\nKp:%.2f\nKi:%.4f\nKd:%.3f",(FLAG_KiKpKd?(FLAG_KiKpKd==1?"Ki":"Kd"):"Kp"),Kp,Ki,Kd);
        lcd_clear_part(LCD_VS_VSA,32,320,240,BLACK);
        lcd_draw_string(LCD_VS_VSA,32,num,WHITE);
        sprintf(num,"Roll:%.2f\nPitch:%.2f",ICM_EndData.roll,ICM_EndData.pitch);
        lcd_draw_string(LCD_VS_VSA,96,num,GREEN);
        EMLOG(LOG_INFO,"YES\n");
    }
    EMLOG(LOG_DEBUG,"No KEY EVENT\n");
}

int main(void){
    uarths_init();
    // uint64_t time[2];
    Hardware_Init();
    lcd_init();
    lcd_VerticalScroll(LCD_VS_TFA,LCD_VS_VSA,LCD_VS_BFA);
    lcd_clear(BLACK);
    EMLOG(LOG_INFO,"\n");
    Key_SM_Init();
    I2C_Read_ADDR();

    lcd_draw_string(LCD_VS_VSA,0,"curve detection:\n",BLUE);
    if( ICM2060_FULL_Init() == 0 ){
        while (1){
            // 通过获取KEY_FIFO的有效事件来处理相关按键任务
            keyevnet_dispose();
            lcd_draw_ICMcurve();
            lcd_draw_RowPoint(LCD_Y_MAX/2,LCD_VS_VSA,GREEN);//绿色0°标线
        }
    }
}
