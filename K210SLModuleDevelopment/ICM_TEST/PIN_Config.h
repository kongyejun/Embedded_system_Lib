#ifndef __PIN_Config__
#define __PIN_Config__
/*****************************HEAR-FILE************************************/

#include "fpioa.h"
#include "i2c.h"

/*****************************HARDWARE-PIN*********************************/
//ICM
#define PIN_ICM_SCLK        (9)
#define PIN_ICM_SDA         (10)
#define PIN_ICM_INT         (11)
//LCD
#define PIN_LCD_CS          (36)
#define PIN_LCD_RST         (37)
#define PIN_LCD_RS          (38)
#define PIN_LCD_WR          (39)
//轮播按键
#define KEYPAD_LIFT         (1)
#define KEYPAD_MID          (2)
#define KEYPAD_RTGHT        (3)

/*****************************SOFTWARE-GPIO********************************/
//ICM
#define ICM_I2CDEVICE       I2C_DEVICE_0
#define ICM_INT_GPIOHSNUM   (0)
//LCD
#define LCD_RST_GPIONUM     (1)
#define LCD_RS_GPIONUM      (2)
//KeyPad
#define KEYL_GPIOHSNUM        (3)
#define KEYM_GPIOHSNUM        (4)
#define KEYR_GPIOHSNUM        (5)

/*****************************FUNC-GPIO************************************/
//ICM
#define FUNC_ICM_SDA    (FUNC_I2C0_SDA + ICM_I2CDEVICE * 2)
#define FUNC_ICM_SCLK   (FUNC_I2C0_SCLK + ICM_I2CDEVICE * 2)
#define FUNC_ICM_INT    (FUNC_GPIOHS0 + ICM_INT_GPIOHSNUM)
//LCD
#define FUNC_LCD_CS     (FUNC_SPI0_SS3)                             // 片选
#define FUNC_LCD_RST    (FUNC_GPIOHS0 + LCD_RST_GPIONUM)            // 复位
#define FUNC_LCD_RS     (FUNC_GPIOHS0 + LCD_RS_GPIONUM)             // 数据/命令(0)
#define FUNC_LCD_WR     (FUNC_SPI0_SCLK)                            // 读/写
//KeyPad
#define FUNC_KEYPAD_L   (FUNC_GPIOHS0 + KEYL_GPIOHSNUM)
#define FUNC_KEYPAD_M   (FUNC_GPIOHS0 + KEYM_GPIOHSNUM)
#define FUNC_KEYPAD_R   (FUNC_GPIOHS0 + KEYR_GPIOHSNUM)

#endif