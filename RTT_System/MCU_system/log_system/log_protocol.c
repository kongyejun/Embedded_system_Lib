#include "log_protocol.h"
#include <string.h>
/*===================================================================
 *                      协议层实现
 *==================================================================*/
/* 空间检查:预留 1 字节给结尾 '\0',不足则跳转 buffer_full */
#define CHECK_SPACE(x) do{ if(frame->len + (x) >= FRAME_PAYLOAD_SIZE){ \
                               goto buffer_full; } }while(0)

/* 初始化协议帧:写入帧头、复位长度与校验和 */
void log_protocol_init(log_frame_t *frame)
{
    frame->head     = FRAME_HEADER;
    frame->len      = 0;
    frame->checksum = 0;
}

/* 格式化写入:解析 fmt,将文本与实参编码进协议帧,返回帧总长度 */
uint8_t log_vsnprintf(log_frame_t *frame, const char *fmt, va_list *args)
{
    uint8_t flage;
    /* 帧已满 */
    if (frame->len >= FRAME_PAYLOAD_SIZE - 1) { goto buffer_full; }
    while (*fmt != '\0')
    {
        /* 普通字符:原样存入 */
        if (*fmt != '%')
        {
            CHECK_SPACE(1);
            log_protocol_put_char(frame, *fmt++);
            continue;
        }
        /* 遇到 '%':先存入(整个格式符只存这一次),再指向下一字符 */
        CHECK_SPACE(1);
        log_protocol_put_char(frame, *fmt++);
        /* "%%" 转义:再存一个 '%' 即可 */
        if (*fmt == '%')
        {
            CHECK_SPACE(1);
            log_protocol_put_char(frame, *fmt++);
            continue;
        }
        /* 解析标志字符 -0+#空格 */
        for (flage = 1; flage;)
        {
            switch (*fmt)
            {
                case '-': 
                case '0': 
                case '+': 
                case '#': 
                case ' ':
                    CHECK_SPACE(1);
                    log_protocol_put_char(frame, *fmt++);
                    break;
                default:
                    flage = 0; break;
            }
        }
        /* 解析宽度 */
        while (*fmt >= '0' && *fmt <= '9')
        {
            CHECK_SPACE(1);
            log_protocol_put_char(frame, *fmt++);
        }
        /* 解析精度:存入 '.' 及后续数字 */
        if (*fmt == '.')
        {
            CHECK_SPACE(1);
            log_protocol_put_char(frame, *fmt++);
            while (*fmt >= '0' && *fmt <= '9')
            {
                CHECK_SPACE(1);
                log_protocol_put_char(frame, *fmt++);
            }
        }
        /* 解析类型符并加载实参('%' 已存,此处只存类型符本身与数据) */
        switch (*fmt)
        {
            case 'c':
                /* 类型符 + 1 字节字符值(char 提升为 int) */
                CHECK_SPACE(2);
                log_protocol_put_char(frame, *fmt);
                log_protocol_put_char(frame, (uint8_t)va_arg(*args, int));
                break;
            case 'd': case 'i':
            case 'x': case 'X':
            case 'u':
            case 'p':
                /* 类型符 + 4 字节整型 */
                CHECK_SPACE(1 + 4);
                log_protocol_put_char(frame, *fmt);
                frame->checksum ^= encode_u32(frame->payload, &frame->len, va_arg(*args, uint32_t));
                break;
            case 'f':
            {
                /* 类型符 + 8 字节 double 位模式(float 已提升为 double) */
                double   dval = va_arg(*args, double);
                uint64_t bits;
                memcpy(&bits, &dval, sizeof(bits));
                CHECK_SPACE(1 + 8);
                log_protocol_put_char(frame, *fmt);
                frame->checksum ^= encode_u64(frame->payload, &frame->len, bits);
                break;
            }
            case 's':
            {
                /* 类型符 + 字符串内容 */
                const char *str = va_arg(*args, const char *);
                if (str == NULL) { str = "(null)"; }
                CHECK_SPACE(1);
                log_protocol_put_char(frame, *fmt);
                while (*str != '\0')
                {
                    CHECK_SPACE(1);
                    log_protocol_put_char(frame, (uint8_t)*str++);
                }
                CHECK_SPACE(1);
                log_protocol_put_char(frame, '\0');
                break;
            }
            default:
                /* 未知类型符:原样保留该字符 */
                CHECK_SPACE(1);
                log_protocol_put_char(frame, *fmt);
                break;
        }
        fmt++;
    }
buffer_full:
    /* CHECK_SPACE 已预留此字节,写入结尾符不会越界 */
    frame->payload[frame->len] = '\0';
    return (int)frame->len;
}