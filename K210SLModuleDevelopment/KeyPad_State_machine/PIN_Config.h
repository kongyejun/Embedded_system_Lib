#ifndef __Pin_Config__
#define __Pin_Config__
#include "fpioa.h"
#include "gpiohs.h"
//硬件引脚定义
#define PIN_KEYM (2)
#define PIN_KEYL (1)
#define PIN_KEYR (3)

//索引号定义
#define KEYM_GPIOHSNUM  (0)
#define KEYL_GPIOHSNUM  (1)
#define KEYR_GPIOHSNUM  (2)

//功能绑定
#define FUNC_KEYM  (FUNC_GPIOHS0 + KEYM_GPIOHSNUM)
#define FUNC_KEYL  (FUNC_GPIOHS0 + KEYL_GPIOHSNUM)
#define FUNC_KEYR  (FUNC_GPIOHS0 + KEYR_GPIOHSNUM)


#endif