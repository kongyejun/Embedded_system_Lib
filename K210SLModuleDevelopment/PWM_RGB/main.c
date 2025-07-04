#include "PIN_FUNC.h"
#include "pwm.h"
#include "sleep.h"

double T0C1_PWM_Init(void){
    double frequency;
    pwm_init(PWM_DEVICE_0);
    frequency = pwm_set_frequency(PWM_DEVICE_0,PWM_CHANNEL_0,200000,0);
    pwm_set_enable(PWM_DEVICE_0,PWM_CHANNEL_0,1);
    return frequency;
}

int main(){
    double duty = 0;uint8_t flage = 0;
    //软硬件绑定
    Hardware_Init();
    //初始化定时器0通道1为PWM模式---------------要求一个周期10us，初始占空比为0
    T0C1_PWM_Init();
    //主程序中，延迟10ms，对占空比进行设置
    while(1){
        msleep(100);
        duty += (flage == 0)?0.1:-0.1;
        if(duty >= 1){flage = 1;duty = 1;}//计算机中的浮点运算不是准确的，只会近似精确    例如：0.8+0.1 = 0.900001
        else if (duty <= 0){flage = 0;duty = 0;}
        pwm_set_frequency(PWM_DEVICE_0,PWM_CHANNEL_0,200,duty);
        if(duty == 0 || (int)duty == 1)
            printf("duty : %f\n",duty);
    }
    return 0;
}