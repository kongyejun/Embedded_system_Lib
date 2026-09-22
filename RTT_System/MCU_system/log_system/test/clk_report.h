#ifndef CLK_REPORT_H
#define CLK_REPORT_H
/**
 * @file clk_report.h
 * @brief STM32H7 外设实际工作时钟检测与上报
 */

#include <stdint.h>

void CLK_Report_All(void);
void CLK_Report_Core(void);
void CLK_Report_Periph(void);

#endif /* CLK_REPORT_H */