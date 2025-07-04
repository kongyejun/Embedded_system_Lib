#ifndef __Pin_Config__
#define __Pin_Config__
#include "fpioa.h"
#include "i2c.h"
/*****************************HARDWARE-PIN*********************************/
//LCD屏
#define PIN_LCD_CS              (36)
#define PIN_LCD_RST             (37)
#define PIN_LCD_RS              (38)
#define PIN_LCD_WR              (39)
//触摸屏
//#define PIN_FT_RST              (37)//与LCD复用
#define PIN_FT_SCL              (9)
#define PIN_FT_SDA              (10)
#define PIN_FT_INT              (12)

//TF卡
#define PIN_TF_MISO            (26)
#define PIN_TF_CLK             (27)
#define PIN_TF_MOSI            (28)
#define PIN_TF_CS              (29)

/*****************************SOFTWARE-GPIO********************************/
//LCD
#define LCD_RST_GPIOHSNUM           (0)
#define LCD_RS_GPIOHSNUM            (1)
//触摸屏
#define FT_INT_GPIOHSNUM            (2)
#define FT_I2C_DEVICENUM            I2C_DEVICE_0
// 软件GPIO口，与程序对应
#define TF_CS_GPIOHSNUM             (3)    

/*****************************FUNC-GPIO************************************/
//LCD
#define FUNC_LCD_CS             (FUNC_SPI0_SS3)                     // 片选
#define FUNC_LCD_RST            (FUNC_GPIOHS0 + LCD_RST_GPIOHSNUM)  // 复位
#define FUNC_LCD_RS             (FUNC_GPIOHS0 + LCD_RS_GPIOHSNUM)   // 数据/命令(0)
#define FUNC_LCD_WR             (FUNC_SPI0_SCLK)                    // 读/写
//触摸屏
#define FUNC_FT_INT            (FUNC_GPIOHS0 + FT_INT_GPIOHSNUM)        // 复位
#define FUNC_FT_SCL            (FUNC_I2C0_SCLK + FT_I2C_DEVICENUM * 2)  // SCL
#define FUNC_FT_SDA            (FUNC_I2C0_SDA + FT_I2C_DEVICENUM * 2)   // SDA
//TF卡
#define FUNC_TF_SPI_MISO        (FUNC_SPI1_D1)
#define FUNC_TF_SPI_CLK         (FUNC_SPI1_SCLK)
#define FUNC_TF_SPI_MOSI        (FUNC_SPI1_D0)
#define FUNC_TF_SPI_CS          (FUNC_GPIOHS0 + TF_CS_GPIOHSNUM)

#endif