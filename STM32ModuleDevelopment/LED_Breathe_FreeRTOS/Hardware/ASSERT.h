#ifndef _USER_ASSERT_H_
#define _USER_ASSERT_H_
#include "stm32f10x.h"
//默认以GPIOA_8作为预警
void ASSERT_Init(void);
void ASSERT_Hapend(void);
#endif
