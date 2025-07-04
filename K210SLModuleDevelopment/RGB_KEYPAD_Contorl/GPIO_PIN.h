#include "fpioa.h"

//外设硬件IO口设置
//RGB灯
#define RGB_R_PIN (6)
#define RGB_G_PIN (7)
#define RGB_B_PIN (8)
//轮播按键
#define KEYPAD_LIFT  (1)
#define KEYPAD_MID   (2)
#define KEYPAD_RTGHT (3)

//软件IO口设置
//RGB灯
#define RGB_R_GPIONUM (0)
#define RGB_G_GPIONUM (1)
#define RGB_B_GPIONUM (2)
//轮播按键
#define KEYL_GPIONUM (3)
#define KEYM_GPIONUM (4)
#define KEYR_GPIONUM (5)

//将软件IO序号与芯片IO口进行配对
#define FUNC_RGB_R (FUNC_GPIO0 + RGB_R_GPIONUM)
#define FUNC_RGB_G (FUNC_GPIO0 + RGB_G_GPIONUM)
#define FUNC_RGB_B (FUNC_GPIO0 + RGB_B_GPIONUM)

#define FUNC_KEYPAD_L (FUNC_GPIO0 + KEYL_GPIONUM)
#define FUNC_KEYPAD_M (FUNC_GPIO0 + KEYM_GPIONUM)
#define FUNC_KEYPAD_R (FUNC_GPIO0 + KEYR_GPIONUM)