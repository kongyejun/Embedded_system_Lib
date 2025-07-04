#ifndef _ST7789_H_
#define _ST7789_H_

#include <stdint.h>
//命令集
#define NO_OPERATION            0x00  // 无操作命令
#define SOFTWARE_RESET          0x01  // 软件复位
#define READ_ID                 0x04  // 读取显示设备的ID
#define READ_STATUS             0x09  // 读取显示状态
#define READ_POWER_MODE         0x0A  // 读取电源模式
#define READ_MADCTL             0x0B  // 读取内存访问控制参数
#define READ_PIXEL_FORMAT       0x0C  // 读取像素格式
#define READ_IMAGE_FORMAT       0x0D  // 读取图像格式
#define READ_SIGNAL_MODE        0x0E  // 读取信号模式
#define READ_SELT_DIAG_RESULT   0x0F  // 读取自检诊断结果
#define SLEEP_ON                0x10  // 进入睡眠模式
#define SLEEP_OFF               0x11  // 退出睡眠模式
#define PARTIAL_DISPLAY_ON      0x12  // 启用部分显示模式
#define NORMAL_DISPLAY_ON       0x13  // 启用正常显示模式
#define INVERSION_DISPLAY_OFF   0x20  // 关闭颜色反转显示
#define INVERSION_DISPLAY_ON    0x21  // 启用颜色反转显示
#define GAMMA_SET               0x26  // 设置伽玛曲线
#define DISPLAY_OFF             0x28  // 关闭显示
#define DISPLAY_ON              0x29  // 打开显示
#define HORIZONTAL_ADDRESS_SET  0x2A  // 设置水平地址范围
#define VERTICAL_ADDRESS_SET    0x2B  // 设置垂直地址范围
#define MEMORY_WRITE            0x2C  // 写入内存
#define COLOR_SET               0x2D  // 设置颜色
#define MEMORY_READ             0x2E  // 读取内存
#define PARTIAL_AREA            0x30  // 设置部分显示区域
#define VERTICAL_SCROLL_DEFINE  0x33  // 定义垂直滚动区域
#define TEAR_EFFECT_LINE_OFF    0x34  // 关闭撕裂效应信号
#define TEAR_EFFECT_LINE_ON     0x35  // 打开撕裂效应信号
#define MEMORY_ACCESS_CTL       0x36  // 内存访问控制
#define VERTICAL_SCROLL_S_ADD   0x37  // 设置垂直滚动起始地址
#define IDLE_MODE_OFF           0x38  // 关闭空闲模式
#define IDLE_MODE_ON            0x39  // 启用空闲模式
#define PIXEL_FORMAT_SET        0x3A  // 设置像素格式
#define WRITE_MEMORY_CONTINUE   0x3C  // 继续写入内存
#define READ_MEMORY_CONTINUE    0x3E  // 继续读取内存
#define SET_TEAR_SCANLINE       0x44  // 设置撕裂扫描线位置
#define GET_SCANLINE            0x45  // 获取当前扫描线
#define WRITE_BRIGHTNESS        0x51  // 写入亮度值
#define READ_BRIGHTNESS         0x52  // 读取亮度值
#define WRITE_CTRL_DISPLAY      0x53  // 写入显示控制参数
#define READ_CTRL_DISPLAY       0x54  // 读取显示控制参数
#define WRITE_BRIGHTNESS_CTL    0x55  // 写入亮度控制
#define READ_BRIGHTNESS_CTL     0x56  // 读取亮度控制
#define WRITE_MIN_BRIGHTNESS    0x5E  // 写入最小亮度值
#define READ_MIN_BRIGHTNESS     0x5F  // 读取最小亮度值
#define READ_ID1                0xDA  // 读取ID1
#define READ_ID2                0xDB  // 读取ID2
#define READ_ID3                0xDC  // 读取ID3
#define RGB_IF_SIGNAL_CTL       0xB0  // RGB接口信号控制
#define NORMAL_FRAME_CTL        0xB1  // 正常帧控制
#define IDLE_FRAME_CTL          0xB2  // 空闲帧控制
#define PARTIAL_FRAME_CTL       0xB3  // 部分显示帧控制
#define INVERSION_CTL           0xB4  // 反转控制
#define BLANK_PORCH_CTL         0xB5  // 空白区控制
#define DISPLAY_FUNCTION_CTL    0xB6  // 显示功能控制
#define ENTRY_MODE_SET          0xB7  // 入口模式设置
#define BACKLIGHT_CTL1          0xB8  // 背光控制1
#define BACKLIGHT_CTL2          0xB9  // 背光控制2
#define BACKLIGHT_CTL3          0xBA  // 背光控制3
#define BACKLIGHT_CTL4          0xBB  // 背光控制4
#define BACKLIGHT_CTL5          0xBC  // 背光控制5
#define BACKLIGHT_CTL7          0xBE  // 背光控制7
#define BACKLIGHT_CTL8          0xBF  // 背光控制8
#define POWER_CTL1              0xC0  // 电源控制1
#define POWER_CTL2              0xC1  // 电源控制2
#define VCOM_CTL1               0xC5  // VCOM控制1
#define VCOM_CTL2               0xC7  // VCOM控制2
#define NV_MEMORY_WRITE         0xD0  // 非易失性存储器写入
#define NV_MEMORY_PROTECT_KEY   0xD1  // 非易失性存储器保护键
#define NV_MEMORY_STATUS_READ   0xD2  // 读取非易失性存储器状态
#define READ_ID4                0xD3  // 读取ID4
#define POSITIVE_GAMMA_CORRECT  0xE0  // 正伽玛校正
#define NEGATIVE_GAMMA_CORRECT  0xE1  // 负伽玛校正
#define DIGITAL_GAMMA_CTL1      0xE2  // 数字伽玛控制1
#define DIGITAL_GAMMA_CTL2      0xE3  // 数字伽玛控制2
#define INTERFACE_CTL           0xF6  // 接口控制

#define LCD_SPIDEVICE           0     //SPI设备号
#define LCD_SPICSNUM            3     //TFT片选信号接在SPI的3号通道的


void tft_hard_init(void);
void tft_write_command(uint8_t cmd);
void tft_write_byte(uint8_t *data_buf, uint32_t length);
void tft_write_half(uint16_t *data_buf, uint32_t length);
void tft_write_word(uint32_t *data_buf, uint32_t length, uint32_t flag);
void tft_fill_data(uint32_t *data_buf, uint32_t length);
#endif
