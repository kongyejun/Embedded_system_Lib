#ifndef PIN_FUNC
#define PIN_FUNC
#include "fpioa.h"
//硬件IO口设定
//RGB灯
#define RGB_R_PIN (6)

//索引号设定
//RGB灯
#define RGB_R_GPIONUM (0)

//索引号功能绑定
#define FUNC_RGB_R (FUNC_TIMER0_TOGGLE1 + RGB_R_GPIONUM)

//硬件软件绑定函数
static void Hardware_Init(void){
    fpioa_set_function(RGB_R_PIN,FUNC_RGB_R);
}
#endif