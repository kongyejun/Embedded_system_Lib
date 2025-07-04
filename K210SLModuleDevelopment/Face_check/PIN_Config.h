#ifndef __PIN_CONFIG_H__
#define __PIN_CONFIG_H__

/*****************************HARDWARE-PIN*********************************/
// 硬件IO口，与原理图对应
#define PIN_LCD_CS             (36)
#define PIN_LCD_RST            (37)
#define PIN_LCD_RS             (38)
#define PIN_LCD_WR             (39)

// camera
#define PIN_DVP_PCLK           (47)
#define PIN_DVP_XCLK           (46)
#define PIN_DVP_HSYNC          (45)
#define PIN_DVP_PWDN           (44)
#define PIN_DVP_VSYNC          (43)
#define PIN_DVP_RST            (42)
#define PIN_DVP_SCL            (41)
#define PIN_DVP_SDA            (40)

/*****************************SOFTWARE-GPIO********************************/
// 软件GPIO口，与程序对应
#define LCD_RST_GPIONUM        (0)
#define LCD_RS_GPIONUM         (1)


/*****************************FUNC-GPIO************************************/
// GPIO口的功能，绑定到硬件IO口
#define FUNC_LCD_CS             (FUNC_SPI0_SS3)
#define FUNC_LCD_RST            (FUNC_GPIOHS0 + LCD_RST_GPIONUM)
#define FUNC_LCD_RS             (FUNC_GPIOHS0 + LCD_RS_GPIONUM)
#define FUNC_LCD_WR             (FUNC_SPI0_SCLK)


#endif