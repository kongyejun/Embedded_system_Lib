#include "log_format.h"

/*===================================================================
 *                      快速转换工具
 *==================================================================*/

uint32_t log_uint_to_dec(char *dst, uint32_t val)
{
    char     tmp[10];                   // uint32 最多 10 位十进制
    uint32_t len = 0u;

    if (val == 0u)
    {
        *dst = '0';
        return 1u;
    }

    while (val != 0u)
    {
        tmp[len++] = (char)('0' + (val % 10u));
        val /= 10u;
    }

    for (uint32_t i = 0u; i < len; i++)
    {
        dst[i] = tmp[len - 1u - i];    // 反转输出
    }

    return len;
}

uint32_t log_str_copy(char *dst, const char *src)
{
    uint32_t len = 0u;

    while (*src != '\0')
    {
        dst[len++] = *src++;
    }

    return len;
}

/*===================================================================
 *                    字符存储 (内部)
 *==================================================================*/

// 写入单个字符, 超出缓冲区时只计数不写入
static void store_char(log_printf_desc_t *d, char c)
{
    if (d->pos + 1u < d->size)
    {
        d->dst[d->pos++] = c;
    }
    d->total_len++;
}

// 写入 N 个相同字符 (用于填充)
static void store_pad(log_printf_desc_t *d, char c, uint32_t n)
{
    while (n-- > 0u)
    {
        store_char(d, c);
    }
}

/*===================================================================
 *                    格式串解析 (内部)
 *==================================================================*/

static uint32_t parse_flags(const char **fmt)
{
    uint32_t f = 0u;

    for (;;)
    {
        switch (**fmt)
        {
            case '-': f |= FMT_FLAG_LEFT_JUSTIFY; (*fmt)++; break;
            case '0': f |= FMT_FLAG_PAD_ZERO;     (*fmt)++; break;
            case '+': f |= FMT_FLAG_PRINT_SIGN;   (*fmt)++; break;
            case '#': f |= FMT_FLAG_ALTERNATE;     (*fmt)++; break;
            default:  return f;
        }
    }
}

static uint32_t parse_width(const char **fmt)
{
    uint32_t w = 0u;

    while (**fmt >= '0' && **fmt <= '9')
    {
        w = w * 10u + ((uint32_t)**fmt - '0');
        (*fmt)++;
    }

    return w;
}

static uint32_t parse_precision(const char **fmt)
{
    uint32_t p = 0u;

    if (**fmt != '.')
    {
        return 0u;
    }

    (*fmt)++;

    while (**fmt >= '0' && **fmt <= '9')
    {
        p = p * 10u + ((uint32_t)**fmt - '0');
        (*fmt)++;
    }

    return p;
}

static log_length_e parse_length(const char **fmt)
{
    if (**fmt == 'l') { (*fmt)++; return LEN_L; }
    if (**fmt == 'h') { (*fmt)++; return LEN_H; }
    return LEN_DEFAULT;
}

/*===================================================================
 *                    参数提取 (内部)
 *==================================================================*/

static int32_t get_signed(va_list *a, log_length_e len)
{
    return (len == LEN_L) ? (int32_t)va_arg(*a, long)
                          : (int32_t)va_arg(*a, int);
}

static uint32_t get_unsigned(va_list *a, log_length_e len)
{
    return (len == LEN_L) ? (uint32_t)va_arg(*a, unsigned long)
                          : (uint32_t)va_arg(*a, unsigned int);
}

/*===================================================================
 *                    数值输出 (内部)
 *==================================================================*/

/**
 * @brief 输出无符号整数
 * @param d     : 格式化描述符
 * @param val   : 数值
 * @param base  : 进制 (10 / 16)
 * @param prec  : 精度 (最少数字位数)
 * @param width : 最小输出宽度
 * @param flags : 格式标志
 * @param upper : 1=大写十六进制
 */
static void print_unsigned(log_printf_desc_t *d,
                           uint32_t val, uint32_t base,
                           uint32_t prec, uint32_t width,
                           uint32_t flags, int upper)
{
    char       tmp[12];                 // 32位最多10位十进制 + 余量
    uint32_t   len = 0u;
    const char *dig = upper ? "0123456789ABCDEF"
                            : "0123456789abcdef";

    //1. 数字转字符 (逆序存入 tmp)
    if (val == 0u)
    {
        tmp[len++] = '0';
    }
    else
    {
        while (val != 0u)
        {
            tmp[len++] = dig[val % base];
            val /= base;
        }
    }

    //2. 精度补零
    while (len < prec)
    {
        tmp[len++] = '0';
    }

    //3. 计算填充
    uint32_t total = len;
    char     pad   = (flags & FMT_FLAG_PAD_ZERO) ? '0' : ' ';

    if (flags & FMT_FLAG_LEFT_JUSTIFY)
    {
        pad = ' ';                      // 左对齐不做零填充
    }

    //4. 右对齐填充
    if (!(flags & FMT_FLAG_LEFT_JUSTIFY) && width > total)
    {
        store_pad(d, pad, width - total);
    }

    //5. 数字输出 (反转)
    for (uint32_t i = len; i > 0u; i--)
    {
        store_char(d, tmp[i - 1u]);
    }

    //6. 左对齐后填充
    if ((flags & FMT_FLAG_LEFT_JUSTIFY) && width > total)
    {
        store_pad(d, ' ', width - total);
    }
}

/**
 * @brief 输出有符号整数
 * @param d     : 格式化描述符
 * @param val   : 数值
 * @param base  : 进制
 * @param prec  : 精度
 * @param width : 最小宽度
 * @param flags : 格式标志
 */
static void print_signed(log_printf_desc_t *d,
                          int32_t val, uint32_t base,
                          uint32_t prec, uint32_t width,
                          uint32_t flags)
{
    uint32_t uval;

    if (val < 0)
    {
        store_char(d, '-');
        uval = (uint32_t)(-(val + 1)) + 1u;  // 安全取绝对值, 避免 INT32_MIN 溢出
        if (width > 0u) width--;
    }
    else
    {
        if (flags & FMT_FLAG_PRINT_SIGN)
        {
            store_char(d, '+');
            if (width > 0u) width--;
        }
        uval = (uint32_t)val;
    }

    print_unsigned(d, uval, base, prec, width,
                   flags & ~FMT_FLAG_PRINT_SIGN, 0);
}

/*===================================================================
 *                    log_vsnprintf 主函数
 *==================================================================*/

int log_vsnprintf(char *dst, uint32_t dst_size,
                  const char *fmt, va_list *args)
{
    log_printf_desc_t d;

    d.dst       = dst;
    d.size      = dst_size;
    d.pos       = 0u;
    d.total_len = 0;

    while (*fmt != '\0')
    {
        //1. 普通字符直接存储
        if (*fmt != '%')
        {
            store_char(&d, *fmt++);
            continue;
        }

        fmt++;                          // 跳过 '%'

        //2. 解析格式修饰符
        uint32_t     flags = parse_flags(&fmt);
        uint32_t     width = parse_width(&fmt);
        uint32_t     prec  = parse_precision(&fmt);
        log_length_e len   = parse_length(&fmt);

        //3. 按说明符分发
        switch (*fmt)
        {
            case 'c':
                store_char(&d, (char)va_arg(*args, int));
                break;

            case 'd':
            case 'i':
                print_signed(&d, get_signed(args, len),
                             10u, prec, width, flags);
                break;

            case 'u':
                print_unsigned(&d, get_unsigned(args, len),
                               10u, prec, width, flags, 0);
                break;

            case 'x':
                print_unsigned(&d, get_unsigned(args, len),
                               16u, prec, width, flags, 0);
                break;

            case 'X':
                print_unsigned(&d, get_unsigned(args, len),
                               16u, prec, width, flags, 1);
                break;

            case 's':
            {
                const char *s = va_arg(*args, const char *);
                if (s == NULL) s = "(null)";
                while (*s != '\0') store_char(&d, *s++);
                break;
            }

            case 'p':
                store_char(&d, '0');
                store_char(&d, 'x');
                print_unsigned(&d, (uint32_t)(uintptr_t)va_arg(*args, void *),
                               16u, 8u, 8u, FMT_FLAG_PAD_ZERO, 0);
                break;

            case '%':
                store_char(&d, '%');
                break;

            default:
                store_char(&d, '%');
                store_char(&d, *fmt);
                break;
        }

        if (*fmt != '\0') fmt++;
    }

    //4. NUL 结尾
    if (dst_size != 0u)
    {
        uint32_t end = (d.pos < dst_size) ? d.pos : (dst_size - 1u);
        dst[end] = '\0';
    }

    return d.total_len;
}