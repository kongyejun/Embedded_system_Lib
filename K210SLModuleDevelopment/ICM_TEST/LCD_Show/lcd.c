#include <string.h>
#include <unistd.h>
#include "GUI\LCD_SHOW\lcd.h"
#include "st7789.h"
#include "font.h"
#include "stdlib.h"

static lcd_ctl_t lcd_ctl;

/* 初始化LCD，设置显示方向和启动显示 */
void lcd_init(void){
    uint8_t data = 0;
    /* 硬件初始化 */
    tft_hard_init();
    /* 重置LCD */
    tft_write_command(SOFTWARE_RESET);
    usleep(100000);
    /* 关闭睡眠模式 */
    tft_write_command(SLEEP_OFF);
    usleep(100000);
    /* 设置像素格式：65K, 16bit/pixel */
    tft_write_command(PIXEL_FORMAT_SET);
    data = 0x55;  /* 0101 0101*/
    tft_write_byte(&data, 1);
    /* 打开显示反转 */
    tft_write_command(INVERSION_DISPLAY_ON);
    /* 设置LCD显示方向 */
    lcd_set_direction(DIR_YX_LRUD);

    /* 使能显示 */
    tft_write_command(DISPLAY_ON);
    /* 清空显示 */
    lcd_clear(WHITE);
}

/* 设置LCD显示方向 */
void lcd_set_direction(lcd_dir_t dir)
{
    lcd_ctl.dir = dir;
    if (dir & DIR_XY_MASK)
    {
        lcd_ctl.width = LCD_Y_MAX - 1;
        lcd_ctl.height = LCD_X_MAX - 1;
    }
    else
    {
        lcd_ctl.width = LCD_X_MAX - 1;
        lcd_ctl.height = LCD_Y_MAX - 1;
    }

    tft_write_command(MEMORY_ACCESS_CTL);
    tft_write_byte((uint8_t *)&dir, 1);
}

/* 设置显示区域 */
void lcd_set_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint8_t data[4] = {0};

    data[0] = (uint8_t)(x1 >> 8);
    data[1] = (uint8_t)(x1);
    data[2] = (uint8_t)(x2 >> 8);
    data[3] = (uint8_t)(x2);
    tft_write_command(HORIZONTAL_ADDRESS_SET);
    tft_write_byte(data, 4);

    data[0] = (uint8_t)(y1 >> 8);
    data[1] = (uint8_t)(y1);
    data[2] = (uint8_t)(y2 >> 8);
    data[3] = (uint8_t)(y2);
    tft_write_command(VERTICAL_ADDRESS_SET);
    tft_write_byte(data, 4);

    tft_write_command(MEMORY_WRITE);
}

/* 设置显示某个点的颜色 */
void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color)
{
    lcd_set_area(x, y, x, y);
    tft_write_half(&color, 1);
}

/* LCD显示字符 */
void lcd_draw_char(uint16_t x, uint16_t y, char c, uint16_t color)
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t data = 0;

    for (i = 0; i < 16; i++)
    {
        data = ascii0816[c * 16 + i];
        for (j = 0; j < 8; j++)
        {
            if (data & 0x80)
                lcd_draw_point(x + j, y, color);
            data <<= 1;
        }
        y++;
    }
}

/* LCD显示字符串 */
void lcd_draw_string(uint16_t x, uint16_t y, char *str, uint16_t color)
{
    while (*str)
    {
        lcd_draw_char(x, y, *str, color);
        str++;
        x += 8;
    }
}

/* 清除屏幕显示 */
void lcd_clear(uint16_t color)
{
    uint32_t data = ((uint32_t)color << 16) | (uint32_t)color;

    lcd_set_area(0, 0, lcd_ctl.height, lcd_ctl.width);
    tft_fill_data(&data, LCD_X_MAX * LCD_Y_MAX / 2);
}

/* LCD画方框 */
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t width, uint16_t color)
{
    uint32_t data_buf[640] = {0};
    uint32_t *p = data_buf;
    uint32_t data = color;//data 保存颜色值 color
    uint32_t index = 0;

    data = (data << 16) | data;//将16位颜色数据扩展为32位，这样每次可以填充两个像素
    for (index = 0; index < 160 * width; index++)//因为8位显示一个像素，16位的data代表2个像素赋值给P。
                                                 //则160代表160*2个像素，width代表线宽：max = 4
        *p++ = data;

    lcd_set_area(x1, y1, x2, y1 + width - 1);
    tft_write_word(data_buf, ((x2 - x1 + 1) * width + 1) / 2, 0);
    lcd_set_area(x1, y2 - width + 1, x2, y2);
    tft_write_word(data_buf, ((x2 - x1 + 1) * width + 1) / 2, 0);
    lcd_set_area(x1, y1, x1 + width - 1, y2);
    tft_write_word(data_buf, ((y2 - y1 + 1) * width + 1) / 2, 0);
    lcd_set_area(x2 - width + 1, y1, x2, y2);
    tft_write_word(data_buf, ((y2 - y1 + 1) * width + 1) / 2, 0);
}

/* LCD画图片*/
void lcd_draw_picture(uint16_t x1, uint16_t y1, uint16_t width, uint16_t height, uint32_t *ptr)
{
    lcd_set_area(x1, y1, x1 + width - 1, y1 + height - 1);
    tft_write_word(ptr, width * height / 2, 0);//注意图片数据是32位的，一次性设置2个像素，故总共的像素点为 Width * Height / 2 
}

/* LCD画图片 */
void lcd_draw_picture_half(uint16_t x1, uint16_t y1, uint16_t width, uint16_t height, uint16_t *ptr)
{
    lcd_set_area(x1, y1, x1 + width - 1, y1 + height - 1);
    tft_write_half(ptr, width * height);//注意图片数据此处为16位
}

/* LCD画图片 */
void lcd_draw_picture_byte(uint16_t x1, uint16_t y1, uint16_t width, uint16_t height, uint8_t *ptr)
{
    lcd_set_area(x1, y1, x1 + width - 1, y1 + height - 1);
    tft_write_byte(ptr, width * height * 2);//注意图片数据此处为8位
}

void lcd_fill_RATE(uint8_t x, uint8_t y,uint16_t w,uint16_t h,uint32_t *data_buf, uint32_t length){
    lcd_set_area(x,y,x+h,y+w);
    tft_fill_data(data_buf,length);
}
