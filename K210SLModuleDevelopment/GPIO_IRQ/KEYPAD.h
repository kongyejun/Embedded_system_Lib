#ifndef KEYPAD
#define KEYPAD
#include "PIN_FUNC.h"
#include "gpiohs.h"
#include "sysctl.h"
#include "plic.h"
#include "TIME_K210.h"

void KEYPAD_Init(void);
int KEYPADL_ISR(void* ctx);
int KEYPADM_ISR(void* ctx);
int KEYPADR_ISR(void* ctx);
#endif