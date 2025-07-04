#ifndef __Pin_Config__
#define __Pin_Config__
#include "fpioa.h"
#include "i2c.h"
/*****************************HARDWARE-PIN*********************************/
#define PIN_OLED_SDA (1)
#define PIN_OLED_SCL (2)

/*****************************SOFTWARE-GPIO********************************/
#define OLED_SDA_GPIONUM (1)
#define OLED_SCL_GPIONUM (2)

/*****************************FUNC-GPIO************************************/
#define FUNC_OLED_SDA  (OLED_SDA_GPIONUM + FUNC_GPIO0)
#define FUNC_OLED_SCL  (OLED_SCL_GPIONUM + FUNC_GPIO0)

#endif