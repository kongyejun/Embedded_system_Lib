#ifndef LOG_PROTOCOL_H
#define LOG_PROTOCOL_H

#include <stdint.h>
#include "log_system.h"
/*===================================================================
 *                      协议常量定义
 *==================================================================*/
#define FRAME_HEADER        0xAA                                 // 帧头
#define FRAME_HEADER_SIZE   3                                    // 帧头固定占用字节数
#define FRAME_MAX_SIZE      SEGGER_RTT_PRINTF_BUFFER_SIZE        // 单次发送最大字节数
#define FRAME_PAYLOAD_SIZE  (FRAME_MAX_SIZE - FRAME_HEADER_SIZE) // 实际可用空间

/**
 * @brief 日志帧结构（联合体设计）
 *
 * @note 使用 union 可以同时访问：
 *       - raw[]：原始字节数组（用于传输）
 *       - 结构体字段：协议字段（用于操作）
 */
typedef union
{
    uint8_t raw[FRAME_MAX_SIZE];
    struct __attribute__((packed))
    {
        uint8_t head;                      // 帧头：0xAA
        uint8_t len;                       // 总长度
        uint8_t checksum;                  // 校验和（XOR）
        uint8_t payload[FRAME_PAYLOAD_SIZE]; // 有效载荷
    };
} log_frame_t;

static inline uint8_t encode_u32(uint8_t *dst, uint8_t *pos, uint32_t val)
{
    uint8_t xor = 0;
    for (int i = 24; i >= 0; i -= 8)
    {
        dst[*pos] = (uint8_t)((val >> i) & 0xFFu);
        xor ^= dst[(*pos)++];   // 对：自增 *pos
    }
    return xor;
}
static inline uint8_t encode_u64(uint8_t *dst, uint8_t *pos, uint64_t val)
{
    uint8_t xor = 0;
    for (int i = 56; i >= 0; i -= 8)
    {
        dst[(*pos)] = (uint8_t)((val >> i) & 0xFFu);
        xor ^= dst[(*pos)++];   // 对：自增 *pos
    }
    return xor;
}
static inline void log_protocol_put_char(log_frame_t *frame, uint8_t c)
{
    frame->payload[frame->len++] = c;
    frame->checksum ^= c;
}

void log_protocol_init(log_frame_t *frame);
uint8_t log_vsnprintf(log_frame_t *frame, const char *fmt, va_list *args);
#endif