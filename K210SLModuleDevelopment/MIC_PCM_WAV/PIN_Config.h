#ifndef __Pin_Config__
#define __Pin_Config__
#include "fpioa.h"
/*****************************HARDWARE-PIN*********************************/
//KEYPAD
#define PIN_KEYPAD_LEFT       (1)
#define PIN_KEYPAD_MIDDLE     (2)
#define PIN_KEYPAD_RIGHT      (3)
//RGB
#define PIN_RGB_R             (6)
#define PIN_RGB_G             (7)
#define PIN_RGB_B             (8)
//TF
#define PIN_TF_MISO           (26)
#define PIN_TF_CLK            (27)
#define PIN_TF_MOSI           (28)
#define PIN_TF_CS             (29)
//扬声器
#define PIN_SPK_WS            (30)
#define PIN_SPK_DA            (31)
#define PIN_SPK_BCK           (32)
//麦克风
#define PIN_MIC_WS            (33)
#define PIN_MIC_DATA          (34)
#define PIN_MIC_SCK           (35)
/*****************************SOFTWARE-GPIO********************************/
// 软件GPIO口，与程序对应
#define TF_CS_GPIOHSNUM        (0)    
//KEYPAD
#define KEYPAD_LEFT_GPIONUM    (1)
#define KEYPAD_MIDDLE_GPIONUM  (2)
#define KEYPAD_RIGHT_GPIONUM   (3)
//RGB
#define RGB_R_GPIONUM          (4)
#define RGB_G_GPIONUM          (5)
#define RGB_B_GPIONUM          (6)
/*****************************FUNC-GPIO************************************/
//TF卡
#define FUNC_TF_SPI_MISO        (FUNC_SPI1_D1)
#define FUNC_TF_SPI_CLK         (FUNC_SPI1_SCLK)
#define FUNC_TF_SPI_MOSI        (FUNC_SPI1_D0)
#define FUNC_TF_SPI_CS          (FUNC_GPIOHS0 + TF_CS_GPIOHSNUM)
//MIC
#define FUNC_MIC_WS             (FUNC_I2S0_WS)
#define FUNC_MIC_DATA           (FUNC_I2S0_IN_D0)//麦克风是输入！！！！！！！！
#define FUNC_MIC_SCK            (FUNC_I2S0_SCLK)
//扬声器 
#define FUNC_SPK_WS             (FUNC_I2S2_WS)
#define FUNC_SPK_DA             (FUNC_I2S2_OUT_D0)//扬声器是输出！！！！！！！！
#define FUNC_SPK_BCK            (FUNC_I2S2_SCLK)
//KEYPAD
#define FUNC_KEYPAD_LEFT        (FUNC_GPIOHS0 + KEYPAD_LEFT_GPIONUM)
#define FUNC_KEYPAD_MIDDLE      (FUNC_GPIOHS0 + KEYPAD_MIDDLE_GPIONUM)
#define FUNC_KEYPAD_RIGHT       (FUNC_GPIOHS0 + KEYPAD_RIGHT_GPIONUM)
//RGB
#define FUNC_RGB_R              (FUNC_GPIOHS0 + RGB_R_GPIONUM)
#define FUNC_RGB_G              (FUNC_GPIOHS0 + RGB_G_GPIONUM)
#define FUNC_RGB_B              (FUNC_GPIOHS0 + RGB_B_GPIONUM)

#endif