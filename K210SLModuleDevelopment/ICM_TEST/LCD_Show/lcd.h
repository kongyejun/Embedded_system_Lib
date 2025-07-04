#ifndef _LCD_H_
#define _LCD_H_

#include <stdint.h>

#define LCD_X_MAX   (320)
#define LCD_Y_MAX   (240)

#define BLACK       0x0000
#define NAVY        0x000F
#define DARKGREEN   0x03E0
#define DARKCYAN    0x03EF
#define MAROON      0x7800
#define PURPLE      0x780F
#define OLIVE       0x7BE0
#define LIGHTGREY   0xC618
#define DARKGREY    0x7BEF
#define BLUE        0x001F
#define GREEN       0x07E0
#define CYAN        0x07FF
#define RED         0xF800
#define MAGENTA     0xF81F
#define YELLOW      0xFFE0
#define WHITE       0xFFFF
#define ORANGE      0xFD20
#define GREENYELLOW 0xAFE5
#define PINK        0xF81F
#define USER_COLOR  0xAA55

/* LCD显示方向枚举类型 */
typedef enum _lcd_dir
{
    DIR_XY_RLUD = 0x00,  // X轴为主轴，显示方向：从右到左，从上到下
    DIR_YX_RLUD = 0x20,  // Y轴为主轴，显示方向：从右到左，从上到下
    DIR_XY_LRUD = 0x40,  // X轴为主轴，显示方向：从左到右，从上到下
    DIR_YX_LRUD = 0x60,  // Y轴为主轴，显示方向：从左到右，从上到下
    DIR_XY_RLDU = 0x80,  // X轴为主轴，显示方向：从右到左，从下到上
    DIR_YX_RLDU = 0xA0,  // Y轴为主轴，显示方向：从右到左，从下到上
    DIR_XY_LRDU = 0xC0,  // X轴为主轴，显示方向：从左到右，从下到上
    DIR_YX_LRDU = 0xE0,  // Y轴为主轴，显示方向：从左到右，从下到上
    DIR_XY_MASK = 0x20,  // X/Y轴主轴选择掩码，用于判断显示方向
    DIR_MASK = 0xE0,     // 方向选择掩码，用于从枚举值中提取方向信息
} lcd_dir_t;

/* LCD结构体 */
typedef struct _lcd_ctl
{
    uint8_t dir;
    uint16_t width;
    uint16_t height;
} lcd_ctl_t;


void lcd_init(void);
void lcd_clear(uint16_t color);
void lcd_set_direction(lcd_dir_t dir);
void lcd_set_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color);
void lcd_draw_string(uint16_t x, uint16_t y, char *str, uint16_t color);
void lcd_draw_picture(uint16_t x1, uint16_t y1, uint16_t width, uint16_t height, uint32_t *ptr);
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t width, uint16_t color);
void lcd_draw_picture_half(uint16_t x1, uint16_t y1, uint16_t width, uint16_t height, uint16_t *ptr);
void lcd_draw_picture_byte(uint16_t x1, uint16_t y1, uint16_t width, uint16_t height, uint8_t *ptr);
void lcd_fill_RATE(uint8_t x, uint8_t y,uint16_t w,uint16_t h,uint32_t *data_buf, uint32_t length);
void lcd_RATE_draw(uint8_t x1, uint8_t y1,uint16_t x2,uint16_t y2,uint32_t *data_buf, uint32_t length);
#endif
