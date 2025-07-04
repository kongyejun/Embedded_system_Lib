#include "FT6236.h"
#include "Pin_Config.h"
#include "gpio_common.h"
#include "gpiohs.h"
#include "gpio.h"
#include "sysctl.h"
#include "sleep.h"
#include "printf.h"
struct FT6236_Point FT_Point1,FT_Point2;
//实现中断函数--------------------------------------中断函数很容易被编译器优化掉！！！！！！！！！！！！！！
int __attribute__((optimize("O0")))FT6236_ISR_Callback(void* ctx){
    return 0;
}

void FT6236_Hardware_Init(void* ctx){
#ifdef PIN_FT_RST
    //使用RST引脚使FT6236复位
    gpio_set_drive_mode(FT_RST_GPIONUM,GPIO_DM_OUTPUT);
    gpio_set_pin(FT_RST_GPIONUM,GPIO_PV_LOW);
    msleep(50);
    gpio_set_pin(FT_RST_GPIONUM,GPIO_PV_HIGH);
    msleep(120);
#endif
    //设置FT的INT引脚（用于中断表示有触摸）
    gpiohs_set_drive_mode(FT_INT_GPIONUM,GPIO_DM_INPUT);
    gpiohs_set_pin_edge(FT_INT_GPIONUM,GPIO_PE_RISING);
    gpiohs_irq_register(FT_INT_GPIONUM,FT6236_IRQ_LEVEL,(plic_irq_callback_t)FT6236_ISR_Callback,ctx);
    msleep(5);
}

void FT_I2C_Write(uint8_t reg,uint8_t data){
    I2C_Write_Byte(FT6236_I2C_ADDR,reg,data);
}

void FT_I2C_Read(uint8_t reg,uint8_t* BUFF,uint8_t Length){
    I2C_Read(FT6236_I2C_ADDR,reg,BUFF,Length);
}

void FT6236_Init(){
    //初始化点结构体
    FT_Point1.state=0;FT_Point1.x=0;FT_Point1.y=0;
    FT_Point2.state=0;FT_Point2.x=0;FT_Point2.y=0;
    //初始化FT硬件配置
    FT6236_Hardware_Init(NULL);
                                        //FT6236设置
    //初始化I2C配置
    I2C_HardWare_Init(FT6236_I2C_ADDR);
    //设置工作模式
    FT_I2C_Write(FT_DEVIDE_MODE, 0x00);
    FT_I2C_Write(FT_ID_G_MODE,0x01);
    /* 设置触摸有效值，越小越灵敏，def=0xbb */
    FT_I2C_Write(FT_ID_G_THGROUP, 0x12);    // 0x22
    /* 工作扫描周期，用于控制报点率，def=0x08, 0x04~0x14 */
    FT_I2C_Write(FT_ID_G_PERIODACTIVE, 0x06);
    msleep(5);
}

void Ponit_Get(int state,struct FT6236_Point *FT_Point){
    uint8_t data[4];
    if(FT_Point == &FT_Point1)FT_Point1.state =  ~(0xFF << (state & 0x0F));
    else FT_Point2.state =  ~(0xFF << (state & 0x0F));
    FT_I2C_Read(FT_TP1_REG, data, 4);
    uint16_t y = ((uint16_t)(data[0] & 0x0f) << 8) + data[1];
    FT_Point->y = (uint16_t)(240 - y);
    FT_Point->x = ((uint16_t)(data[2] & 0x0f) << 8) + data[3];
    if ((data[0] & 0xC0) != 0x80){
        //0x03与0x09中最高2位，代表点的状态
        //0x00: 按下 | 0x01: 抬起 | 0x10: 接触 | 0x11: 无事件(滑动之类的) 
        //对于FT6236来说,检测到"按下"触摸点被确认有效并且用户继续保持触摸时改为"接触"状态
        FT_Point->x = FT_Point->y = 0;
        return;
    }
    FT_Point->state |= TP_PRES_DOWN;
}


void FT6236_GetPXY(){
    uint8_t state;
    //读取点数量寄存器
    FT_I2C_Read(FT_REG_NUM_FINGER,&state,1);
    //如果存在触摸
    if(state & 0x0F){
        if(state&0x01)Ponit_Get(state,&FT_Point1);
        if(state&0x02)Ponit_Get(state,&FT_Point2);
    }else{
        if(FT_Point1.state & TP_PRES_DOWN){FT_Point1.state &= ~0x80;}
        else{FT_Point1.state&=0xe0;FT_Point1.x=0;FT_Point1.y=0;}
        
        if(FT_Point2.state & TP_PRES_DOWN){FT_Point2.state &= ~0x80;}
        else{FT_Point2.state&=0xe0;FT_Point2.x=0;FT_Point2.y=0;}
    }
}
