#ifndef __PING_CONFIG__
#define __PING_CONFIG__
#include "fpioa.h"

//硬件IO口定义
//扬声器
#define PIN_SPK_WS             (30)
#define PIN_SPK_DATA           (31)
#define PIN_SPK_SCK            (32)
//麦克风
#define PIN_MIC_WS             (33)
#define PIN_MIC_DATA           (34)
#define PIN_MIC_SCK            (35)
//索引号定义

//功能绑定定义
#define FUNC_SPK_WS         FUNC_I2S1_WS
#define FUNC_SPK_DATA       FUNC_I2S1_OUT_D0
#define FUNC_SPK_SCK        FUNC_I2S1_SCLK

#define FUNC_MIC_WS         FUNC_I2S2_WS
#define FUNC_MIC_DATA       FUNC_I2S2_IN_D0
#define FUNC_MIC_SCK        FUNC_I2S2_SCLK

#endif