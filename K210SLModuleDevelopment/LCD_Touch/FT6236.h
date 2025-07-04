#ifndef FT6236
#define FT6236
#include "i2c_ctl.h"

#define FT6236_I2C_ADDR         0x38

#define FT6236_IRQ_LEVEL        1

#define TP_PRES_DOWN            0x80  //触屏被按下	
#define TP_COORD_UD             0x40  //触摸坐标更新标记

/* FT6236 部分寄存器定义 */
#define FT_DEVIDE_MODE 			0x00   		//FT6236模式控制寄存器
#define FT_REG_NUM_FINGER       0x02		//触摸状态寄存器

#define FT_TP1_REG 				0X03	  	//第一个触摸点数据地址
#define FT_TP2_REG 				0X09		//第二个触摸点数据地址

#define FT_ID_G_MODE 			0xA4   		//FT6236中断模式控制寄存器
#define FT_ID_G_THGROUP			0x80   		//触摸有效值设置寄存器
#define FT_ID_G_PERIODACTIVE	0x88   		//激活状态周期设置寄存器  

#define FT_DMA_CHANNE DMAC_CHANNEL0

//定义一个点的状态
struct FT6236_Point{
    uint8_t state;
    // 触摸状态 b7:按下1/松开0; b6:0没有按键按下/1有按键按下;
    // bit5-bit1:保留；bit0触摸点按下有效标志，有效为1
    uint16_t x;
    uint16_t y;
};

extern struct FT6236_Point FT_Point1,FT_Point2;

//FT6236初始化函数
void FT6236_Init();
//FT6236发送数据
void FT_I2C_Write(uint8_t reg,uint8_t data);
//FT6236接收数据
void FT_I2C_Read(uint8_t reg,uint8_t* BUFF,uint8_t Length);
//FT6236获取有效触摸点XY
void FT6236_GetPXY();
#endif
