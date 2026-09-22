#include "log_system.h"
#include "log_protocol.h"
#include <string.h>
#include <intrinsics.h>
/*===================================================================
 *                          类型与变量声明
 *==================================================================*/

// 系统结构体(便于扩展)
 typedef struct
{
    SEGGER_RTT_CB     *rtt;             // RTT 控制块
    uint32_t          drop_count;       // 缓冲区溢出丢弃计数
} log_system_t;

static log_system_t log_sys;

/*===================================================================
 *                          外部API
 *==================================================================*/
// 初始化日志系统
void log_system_init(void)
{
    // 配置SEGGER
    SEGGER_RTT_Init();
    SEGGER_RTT_ConfigUpBuffer(0, "LOG", NULL, NULL, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
    // 配置log_system
    memset(&log_sys,0,sizeof(log_sys));
    log_sys.rtt = &_SEGGER_RTT;
    LOG_INFO("log init ok, buf=%u, filter=%d\n", log_sys.rtt->aUp->SizeOfBuffer, LOG_LEVEL_FILTER);
}

// 数据输出
uint8_t log_output(uint8_t lv_char, const char *func, uint32_t line_num, const char *fmt, ...)
{
    log_frame_t frame;
    va_list args;
    // 2. 初始化协议层
    log_protocol_init(&frame);
    // 3. 写入前缀: {level}{func}{line}:
    log_protocol_put_char(&frame,lv_char);
    do
    {
        log_protocol_put_char(&frame,*func);
    }while(*(++func) != '\0');
    log_protocol_put_char(&frame,':');  
    // 4. 格式化用户内容（边写边算校验和）
    va_start(args, fmt);
    log_vsnprintf(&frame, fmt, &args);
    va_end(args);
    // 5. 写入内存中
    if (frame.len <= SEGGER_RTT_GetAvailWriteSpace(0))
    {
        SEGGER_RTT_Write(0, frame.raw, frame.len + FRAME_HEADER_SIZE);
    }
    else
    {
        log_sys.drop_count += 1;  // 溢出计数
    }
    return frame.len;
}

// 原始无格式数据输出
uint8_t log_raw(const char *fmt, ...)
{
    log_frame_t frame;
    va_list args;
    // 2. 初始化协议层
    log_protocol_init(&frame);
    // 3. 格式化用户内容（边写边算校验和）
    va_start(args, fmt);
    log_vsnprintf(&frame, fmt, &args);
    va_end(args);
    // 4. 写入内存中
    if (frame.len <= SEGGER_RTT_GetAvailWriteSpace(0))
    {
        SEGGER_RTT_Write(0, frame.raw, frame.len + FRAME_HEADER_SIZE);
    }
    else
    {
        log_sys.drop_count += 1;  // 溢出计数
    }
    return frame.len;
}

// 空闲空间不足上报
void log_free_space_report(void)
{
    uint32_t free  = SEGGER_RTT_GetAvailWriteSpace(0);
    if(free < 25)
    {
        LOG_WARN("RTT Not Enough: Drops=%d,Free=%d\n",log_sys.drop_count,free);
    }
}
