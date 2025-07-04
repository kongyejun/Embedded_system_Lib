#ifndef __Pin_Config__
#define __Pin_Config__
#include "fpioa.h"
/*****************************HARDWARE-PIN*********************************/
//LCD屏
#define PIN_LCD_CS              (36)
#define PIN_LCD_RST             (37)
#define PIN_LCD_RS              (38)
#define PIN_LCD_WR              (39)
// 轮盘按键
#define PIN_KEYPAD1             (1)
#define PIN_KEYPAD2             (2)
#define PIN_KEYPAD3             (3)

/*****************************SOFTWARE-GPIO********************************/
// 轮盘按键
#define KEYPAD_GPIOHSNUM1           (3)
#define KEYPAD_GPIOHSNUM2           (4)
#define KEYPAD_GPIOHSNUM3           (5)

// LCD
#define LCD_RST_GPIOHSNUM           (1)
#define LCD_RS_GPIOHSNUM            (2)

/*****************************FUNC-GPIO************************************/
// 轮盘按键
#define FUNC_KEYPAD1             (FUNC_GPIOHS0 + KEYPAD_GPIOHSNUM1)
#define FUNC_KEYPAD2             (FUNC_GPIOHS0 + KEYPAD_GPIOHSNUM2)
#define FUNC_KEYPAD3             (FUNC_GPIOHS0 + KEYPAD_GPIOHSNUM3)

// LCD
#define FUNC_LCD_CS             (FUNC_SPI0_SS3)                     // 片选
#define FUNC_LCD_RST            (FUNC_GPIOHS0 + LCD_RST_GPIOHSNUM)  // 复位
#define FUNC_LCD_RS             (FUNC_GPIOHS0 + LCD_RS_GPIOHSNUM)   // 数据/命令(0)
#define FUNC_LCD_WR             (FUNC_SPI0_SCLK)                    // 读/写

#endif