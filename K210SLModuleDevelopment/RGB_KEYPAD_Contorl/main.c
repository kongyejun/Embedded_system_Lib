#include "GPIO_PIN.h"
#include "fpioa.h"
#include "gpio.h"
#include "sleep.h"

//芯片IO口与外设IO口绑定（映射）
void hardware_init(void){
    fpioa_set_function(RGB_R_PIN , FUNC_RGB_R);
    fpioa_set_function(RGB_G_PIN , FUNC_RGB_G);
    fpioa_set_function(RGB_B_PIN , FUNC_RGB_B);

    fpioa_set_function(KEYPAD_LIFT  , FUNC_KEYPAD_L);
    fpioa_set_function(KEYPAD_MID   , FUNC_KEYPAD_M);
    fpioa_set_function(KEYPAD_RTGHT , FUNC_KEYPAD_R);
}

void OFF_RGB(void){
    gpio_set_pin(RGB_R_GPIONUM,GPIO_PV_HIGH);
    gpio_set_pin(RGB_G_GPIONUM,GPIO_PV_HIGH);
    gpio_set_pin(RGB_B_GPIONUM,GPIO_PV_HIGH);
}

//RGB初始化函数
void init_RGB(void){
    gpio_set_drive_mode(RGB_R_GPIONUM,GPIO_DM_OUTPUT);//设置GPIO口对应pin的工作模式
    //为什么不是FUNC_RGB_R而是RGB_R_NUM?   
    //因为这里使用的是gpio函数,内部默认起始地址为GPIO0,传入RGB_R_NUM即偏移量即可确定是哪一个Pin
    gpio_set_drive_mode(RGB_G_GPIONUM,GPIO_DM_OUTPUT);
    gpio_set_drive_mode(RGB_B_GPIONUM,GPIO_DM_OUTPUT);
    //默认RGB灯不工作
    OFF_RGB();
}

void init_KEYPAD(void){
    //设置与按键相关的GPIO
    gpio_set_drive_mode(KEYL_GPIONUM,GPIO_DM_INPUT_PULL_UP);
    gpio_set_drive_mode(KEYM_GPIONUM,GPIO_DM_INPUT_PULL_UP);
    gpio_set_drive_mode(KEYR_GPIONUM,GPIO_DM_INPUT_PULL_UP);
}

//按键扫描
int KEYPAD_SCAN(void){
    while(1){
        if(gpio_get_pin(KEYL_GPIONUM)==GPIO_PV_LOW){
            msleep(10);
            while(!gpio_get_pin(KEYL_GPIONUM));//(如果不消抖，则会很明显出现RGB灯抖动)
            msleep(10);
            return 0;
        }else if(gpio_get_pin(KEYM_GPIONUM)==GPIO_PV_LOW){
            msleep(10);
            while(!gpio_get_pin(KEYM_GPIONUM));
            msleep(10);
            return 1;
        }else if(gpio_get_pin(KEYR_GPIONUM)==GPIO_PV_LOW){
            msleep(10);
            while(!gpio_get_pin(KEYR_GPIONUM));
            msleep(10);
            return 2;
        }
    }
    return -1;
}

int main(void){
    int state = 0;//状态参数
    hardware_init();
    gpio_init();//开启GPIO时钟
    init_RGB();
    while(1){
        state = KEYPAD_SCAN();
        OFF_RGB();
        switch(state){
            case -1:return 0;
            case 0:gpio_set_pin(RGB_R_GPIONUM,GPIO_PV_LOW);break;
            //gpio_set_pin(RGB_R_GPIONUM,0);等价，因为是enum
            case 1:gpio_set_pin(RGB_G_GPIONUM,GPIO_PV_LOW);break;
            case 2:gpio_set_pin(RGB_B_GPIONUM,GPIO_PV_LOW);break;
        }
    }
    return 0;
}

//cd build
//cmake ../ -DPROJ=RGB_KEYPAD_Contorl -G "MinGW Makefiles"
//make