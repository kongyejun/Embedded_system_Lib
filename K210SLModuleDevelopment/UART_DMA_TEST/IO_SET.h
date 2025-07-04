#include "fpioa.h"
#include "uart.h"
//硬件IO口设定     
//UART  (SIP即串口烧录程序,所以USB转串口的硬件IO口为4,5)
#define UART_USB_RX_PIN (4)
#define UART_USB_TX_PIN (5)

//RGB灯
#define RGB_R_PIN (6)
#define RGB_G_PIN (7)
#define RGB_B_PIN (8)

//索引号设定
//UART
#define UART_DEVICENUM UART_DEVICE_3
//RGB灯
#define RGB_R_GPIONUM (0)
#define RGB_G_GPIONUM (1)
#define RGB_B_GPIONUM (2)

//索引号功能绑定
#define FUNC_UART_USB_RX (FUNC_UART1_RX + UART_DEVICENUM * 2)
#define FUNC_UART_USB_TX (FUNC_UART1_TX + UART_DEVICENUM * 2)

#define FUNC_RGB_R (FUNC_GPIO0 + RGB_R_GPIONUM)
#define FUNC_RGB_G (FUNC_GPIO0 + RGB_G_GPIONUM)
#define FUNC_RGB_B (FUNC_GPIO0 + RGB_B_GPIONUM)
