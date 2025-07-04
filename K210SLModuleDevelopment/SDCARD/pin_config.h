#ifndef __PIN_CONFIG_H_
#define __PIN_CONFIG_H_
/*****************************HEAR-FILE************************************/
#include "fpioa.h"

/*****************************HARDWARE-PIN*********************************/
// 硬件IO口，与原理图对应
#define PIN_TF_MISO            (26)
#define PIN_TF_CLK             (27)
#define PIN_TF_MOSI            (28)
#define PIN_TF_CS              (29)

/*****************************SOFTWARE-GPIO********************************/
// 软件GPIO口，与程序对应
#define TF_CS_GPIONUM          (2)      
//对于SPI来说CS信号是由软件实现的,所以硬件上的操作无实际意义,为此CS才会占用一个IO口

/*****************************FUNC-GPIO************************************/
// GPIO口的功能，绑定到硬件IO口
#define FUNC_TF_SPI_MISO        (FUNC_SPI1_D1)
#define FUNC_TF_SPI_CLK         (FUNC_SPI1_SCLK)
#define FUNC_TF_SPI_MOSI        (FUNC_SPI1_D0)
#define FUNC_TF_SPI_CS          (FUNC_GPIOHS0 + TF_CS_GPIONUM)

#endif /* __PIN_CONFIG_H_ */
