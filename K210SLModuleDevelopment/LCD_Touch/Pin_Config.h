#ifndef Pin_Config
#define Pin_Congfig
#include "fpioa.h"
#include "i2c.h"
//硬件IO口设置
//LCD屏
#define PIN_LCD_CS              (36)
#define PIN_LCD_RST             (37)
#define PIN_LCD_RS              (38)
#define PIN_LCD_WR              (39)
//触摸屏
//#define PIN_FT_RST              (37)//与LCD复用
#define PIN_FT_INT              (12)
#define PIN_FT_SCL              (9)
#define PIN_FT_SDA              (10)

//索引号设置
//LCD
#define LCD_RST_GPIONUM         (0)
#define LCD_RS_GPIONUM          (1)
//触摸屏
#define FT_INT_GPIONUM          (2)
#define FT_I2C_DEVICENUM        I2C_DEVICE_0

//功能绑定
//LCD
#define FUNC_LCD_CS             (FUNC_SPI0_SS3)                             // 片选
#define FUNC_LCD_RST            (FUNC_GPIOHS0 + LCD_RST_GPIONUM)            // 复位
#define FUNC_LCD_RS             (FUNC_GPIOHS0 + LCD_RS_GPIONUM)             // 数据/命令(0)
#define FUNC_LCD_WR             (FUNC_SPI0_SCLK)                            // 读/写
//触摸屏
#define FUNC_FT_INT            (FUNC_GPIOHS0 + FT_INT_GPIONUM)              // 复位
#define FUNC_FT_SCL            (FUNC_I2C0_SCLK + FT_I2C_DEVICENUM * 2)      // SCL
#define FUNC_FT_SDA            (FUNC_I2C0_SDA + FT_I2C_DEVICENUM * 2)       // SDA

#endif