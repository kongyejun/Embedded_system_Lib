#ifndef LOG_SYSTEM_H
#define LOG_SYSTEM_H

#include <stdint.h>
#include <stdarg.h>
#include "SEGGER_RTT.h"

/*===================================================================
 *                        日志级别
 *==================================================================*/
#define LOG_LV_DEBUG  0                   // 调试
#define LOG_LV_INFO   1                   // 信息
#define LOG_LV_WARN   2                   // 警告
#define LOG_LV_ERROR  3                   // 错误
#define LOG_LV_NONE   4                   // 全部关闭

/*===================================================================
 *                      编译期裁剪配置
 *==================================================================*/

/* 低于此级别的日志在编译期消除, 不产生任何代码
 * 默认 LOG_LV_DEBUG (全部输出)
 * 设为 LOG_LV_WARN  则 DEBUG/INFO 完全裁掉
 * 设为 LOG_LV_NONE  则所有日志裁掉 */
#define LOG_LEVEL_FILTER    LOG_LV_INFO
/* 1=数据输出开启  0=数据输出裁掉 */
#define LOG_DATA_ENABLE     1

/*===================================================================
 *                        日志宏
 *==================================================================*/

/* ---- 日志 (编译期级别裁剪) ---- */
#if (LOG_LV_DEBUG >= LOG_LEVEL_FILTER)
  #define LOG_DEBUG(fmt, ...)  log_output('D', __func__, __LINE__, fmt, ##__VA_ARGS__)
#else
  #define LOG_DEBUG(fmt, ...)  ((void)0)
#endif

#if (LOG_LV_INFO >= LOG_LEVEL_FILTER)
  #define LOG_INFO(fmt, ...)   log_output('I',  __func__, __LINE__, fmt, ##__VA_ARGS__)
#else
  #define LOG_INFO(fmt, ...)   ((void)0)
#endif

#if (LOG_LV_WARN >= LOG_LEVEL_FILTER)
  #define LOG_WARN(fmt, ...)   log_output('W',  __func__, __LINE__, fmt, ##__VA_ARGS__)
#else
  #define LOG_WARN(fmt, ...)   ((void)0)
#endif

#if (LOG_LV_ERROR >= LOG_LEVEL_FILTER)
  #define LOG_ERROR(fmt, ...)  log_output('E', __func__, __LINE__, fmt, ##__VA_ARGS__)
#else
  #define LOG_ERROR(fmt, ...)  ((void)0)
#endif

/* ---- 数据输出 ---- */
#if LOG_DATA_ENABLE
  #define LOG_DATA(fmt, ...)   log_raw(fmt, ##__VA_ARGS__)
#else
  #define LOG_DATA(fmt, ...)   ((void)0)
#endif

/* ---- 裸输出 (始终可用) ---- */
#define LOG_RAW(fmt, ...)      log_raw(fmt, ##__VA_ARGS__)

/*===================================================================
 *                        公共 API
 *==================================================================*/

/**
 * @brief 日志系统初始化, 清零控制块并注册 RTT 通道
 */
void log_system_init(void);

/**
 * @brief 日志输出, 带完整前缀 {tick}{level}{func}{line}:
 * @param level    : 日志级别
 * @param func     : 调用函数名 (__func__)
 * @param line_num : 调用行号   (__LINE__)
 * @param fmt      : printf 风格格式串
 * @return 传输字节数
 */
uint8_t log_output(uint8_t lv_char, const char *func, uint32_t line_num, const char *fmt, ...);
// 裸输出数据
uint8_t log_raw(const char *fmt, ...);

// 日志系统统计信息上报
void log_free_space_report(void);
#endif /* LOG_SYSTEM_H */