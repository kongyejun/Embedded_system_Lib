#include "PIN_FUNC.h"
#include "LED.h"
#include "KEYPAD.h"
#include "bsp.h"
/*
#注意在K210中：
1.GPIO没有中断资源，GPIOHS才具备中断资源(共有32个这样的IO资源)
2.使用任何外部中断之前都记得开启全局中断.
3.每当使用一种外设时,都记得调用对应的外设初始化函数[ XXXX_init() ]
4.中断程序官方已经包装过,每次触发后会"自动清除中断标志位"
5.关于中断类型如何消抖?
    (1)中断函数中使用延迟消抖;                                           容易造成其他任务阻塞,干扰运行(不建议)
    (2)使用计数变量,只有当变量达到阈值以后才会执行对应程序;                需要结合实际调节阈值,不方便
    (3)中断程序中启用定时器,达到设定时间后进入定时器中断函数执行程序;       需要设备支持定时器(不断使能都会从新计数)

6.一定要注意每一次初始化都会导致通道中的中断函数被清除绑定，所以若要采用当前这种写法一定要避免被重复初始化！！！！！
*/

//软硬件引脚映射
void Hardware_init(void){
    fpioa_init();
    //KEYPAD软硬件绑定
    fpioa_set_function(PIN_KEYPAD_L,FUNC_GPIOHS_KEYPADL);
    fpioa_set_function(PIN_KEYPAD_M,FUNC_GPIOHS_KEYPADM);
    fpioa_set_function(PIN_KEYPAD_R,FUNC_GPIOHS_KEYPADR);
    //LED软硬件绑定
    fpioa_set_function(PIN_RED_LED, FUNC_GPIO_LEDR);
    fpioa_set_function(PIN_GREEN_LED, FUNC_GPIO_LEDG);
}

int main(void){
    Hardware_init();
    LED_Init();
    OFF_LED();
    KEYPAD_Init();
    while(1);
    return 0;
}