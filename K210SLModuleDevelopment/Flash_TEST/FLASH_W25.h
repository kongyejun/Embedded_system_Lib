#ifndef __FLASH_W25_H__
#define __FLASH_W25_H__
#include <stdint.h>
#include "dmac.h"

/*********************************************************
                    是否开启FASFT文件系统
*********************************************************/
#define FLASH_FATFS_OPEN 

/* clang-format off */
#define DATALENGTH                          8  // 数据长度

/* 硬件设备选择 */
//可以考虑设计一个硬件复用树，根据规则检查是否被占用!!!!!!!!
#define SPI_SLAVE_SELECT                       (0x01)  // SPI从机选择
#define W25_FLASH_CS                SPI_CHIP_SELECT_0  // W25QXX片选通道
#define W25_FLASH_SPIDEVICE              SPI_DEVICE_3  //  W25QXX SPI设备
#define W25_FLASH_DMA_SEND              DMAC_CHANNEL0  // W25QXX DMA通道
#define W25_FLASH_DMA_RECEIVE           DMAC_CHANNEL1  // W25QXX DMA通道
/* W25闪存内存分布 */
#define W25_FLASH_PAGE_SIZE              256            // w25qxx闪存页面大小
#define W25_FLASH_SECTOR_SIZE            4096           // w25qxx闪存扇区大小
#define W25_FLASH_PAGE_NUM_PER_SECTOR    16             // w25qxx闪存每个扇区的页面数量
#define W25_FLASH_CHIP_SIZE              (16777216 UL)  // w25qxx闪存芯片大小
/* W25命令表 */
#define W25_WRITE_ENABLE                        0x06    // 写入使能
#define W25_WRITE_DISABLE                       0x04    // 写入禁用
#define W25_READ_REG1                           0x05    // 读取寄存器1
#define W25_READ_REG2                           0x35    // 读取寄存器2
#define W25_READ_REG3                           0x15    // 读取寄存器3
#define W25_WRITE_REG1                          0x01    // 写入寄存器1
#define W25_WRITE_REG2                          0x31    // 写入寄存器2
#define W25_WRITE_REG3                          0x11    // 写入寄存器3
#define W25_READ_DATA                           0x03    // 读取数据
#define W25_FAST_READ                           0x0B    // 快速读取
#define W25_FAST_READ_DUAL_OUTPUT               0x3B    // 快速读取双输出
#define W25_FAST_READ_QUAL_OUTPUT               0x6B    // 快速读取qual输出
#define W25_FAST_READ_DUAL_IO                   0xBB    // 快速读取双I/O
#define W25_FAST_READ_QUAL_IO                   0xEB    // 快速读取qual I/O
#define W25_DUAL_READ_RESET                     0xFFFF  // 双读取重置
#define W25_QUAL_READ_RESET                     0xFF    // qual读取重置
#define W25_PAGE_PROGRAM                        0x02    // 页面编程
#define W25_QUAD_PAGE_PROGRAM                   0x32    // quad页面编程
#define W25_SECTOR_ERASE                        0x20    // 扇区擦除
#define W25_BLOCK_32K_ERASE                     0x52    // 块32K擦除
#define W25_BLOCK_64K_ERASE                     0xD8    // 块64K擦除
#define W25_CHIP_ERASE                          0x60    // 芯片擦除
#define W25_READ_ID                             0x90    // 读取ID
#define W25_ENABLE_QPI                          0x38    // 启用QPI
#define W25_EXIT_QPI                            0xFF    // 退出QPI
#define W25_ENABLE_RESET                        0x66    // 启用重置
#define W25_RESET_DEVICE                        0x99    // 重置设备
/* 寄存器掩码宏定义 */
#define W25_REG1_BUSY_MASK                      0x01  // 寄存器1忙掩码
#define W25_REG2_QUAL_MASK                      0x02  // 寄存器2qual掩码

#define LETOBE(x)     ((x >> 24) | ((x & 0x00FF0000) >> 8) | ((x & 0x0000FF00) << 8) | (x << 24))
/* clang-format on */

/**
 * @brief      w25qxx operating status enumerate
 */
typedef enum _w25qxx_status
{
    W25QXX_OK = 0,
    W25QXX_BUSY,
    W25QXX_ERROR,
} w25qxx_status_t;

/**
 * @brief      w25qxx read operating enumerate
 */
typedef enum _w25qxx_read
{
    W25QXX_STANDARD = 0,
    W25QXX_STANDARD_FAST,
    W25QXX_DUAL,
    W25QXX_DUAL_FAST,
    W25QXX_QUAD,
    W25QXX_QUAD_FAST,
} w25qxx_read_t;

struct W25ID{
    uint8_t manuf_id;
    uint8_t device_id;
};

extern struct W25ID W25FLASH_INFO;

w25qxx_status_t FLASH_W25_Init(void);
w25qxx_status_t FLASH_W25_Write(uint32_t addr, uint8_t *data_buf, uint32_t length);
w25qxx_status_t FLASH_W25_Read(uint32_t addr, uint8_t *data_buf, uint32_t length);
#endif