#ifndef __OLED_I2CPERIPH_H__
#include "stm32f10x.h"

void OLED_Init(void);
void OLED_Clear(void);
void OLED_Clear_8Line(uint8_t Line, uint8_t Line_Num);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowNOWChar16X16(uint8_t Line, uint8_t Column);
void OLED_ShowTargetChar16X16(uint8_t Line, uint8_t Column);
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
#endif
