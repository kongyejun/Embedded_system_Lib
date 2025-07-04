/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _SYSLOG_H
#define _SYSLOG_H

#include <stdint.h>
#include <stdio.h>
#include "printf.h"
#include "encoding.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 日志库
 *
 * 日志库有两种管理日志详细程度的方法：编译时设置，通过 menuconfig 设置。
 *
 * 在编译时，通过使用 `CONFIG_LOG_DEFAULT_LEVEL` 宏进行过滤，该宏通过 `menuconfig` 设置。
 * 所有高于 `CONFIG_LOG_DEFAULT_LEVEL` 的日志语句将由预处理器删除。
 *
 *
 * 如何使用此库：
 *
 * 在每个使用日志功能的 C 文件中，定义 `TAG` 变量如下：
 *
 *      static const char *TAG = "MODULE_NAME";
 *
 * 然后使用其中一个日志宏来生成输出，例如：
 *
 *      LOGW(TAG, "Interrupt error %d", error);
 *
 * 不同的宏可用于不同的日志详细级别：
 *
 *      LOGE - 错误
 *      LOGW - 警告
 *      LOGI - 信息
 *      LOGD - 调试
 *      LOGV - 详细
 *
 * 要在文件或组件范围内覆盖默认的详细级别，请定义 `LOG_LEVEL` 宏。
 * 在文件范围内，定义它并放在 `esp_log.h` 之前，例如：
 *
 *      #define LOG_LEVEL LOG_VERBOSE
 *      #include "dxx_log.h"
 *
 * 在组件范围内，在组件的 Makefile 中定义它：
 *
 *      CFLAGS += -D LOG_LEVEL=LOG_DEBUG
 *
 */

/* clang-format off */
typedef enum _kendryte_log_level
{
    LOG_NONE,       /*!< No log output */
    LOG_ERROR,      /*!< Critical errors, software module can not recover on its own */
    LOG_WARN,       /*!< Error conditions from which recovery measures have been taken */
    LOG_INFO,       /*!< Information messages which describe normal flow of events */
    LOG_DEBUG,      /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    LOG_VERBOSE     /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
} kendryte_log_level_t ;
/* clang-format on */

/* clang-format off */
#if CONFIG_LOG_COLORS
#define LOG_COLOR_BLACK   "30"
#define LOG_COLOR_RED     "31"
#define LOG_COLOR_GREEN   "32"
#define LOG_COLOR_BROWN   "33"
#define LOG_COLOR_BLUE    "34"
#define LOG_COLOR_PURPLE  "35"
#define LOG_COLOR_CYAN    "36"
#define LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
#define LOG_RESET_COLOR   "\033[0m"
#define LOG_COLOR_E       LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W       LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I       LOG_COLOR(LOG_COLOR_GREEN)
#define LOG_COLOR_D
#define LOG_COLOR_V
#else /* CONFIG_LOG_COLORS */
#define LOG_COLOR_E
#define LOG_COLOR_W
#define LOG_COLOR_I
#define LOG_COLOR_D
#define LOG_COLOR_V
#define LOG_RESET_COLOR
#endif /* CONFIG_LOG_COLORS */
/* clang-format on */

#define LOG_FORMAT(letter, format)  LOG_COLOR_ ## letter #letter " (%lu) %s: " format LOG_RESET_COLOR "\n"

#ifdef LOG_LEVEL
#undef CONFIG_LOG_LEVEL
#define CONFIG_LOG_LEVEL LOG_LEVEL
#endif

#ifdef LOG_KERNEL
#define LOG_PRINTF printk
#else
#define LOG_PRINTF printf
#endif

#ifdef CONFIG_LOG_ENABLE
#define LOGE(tag, format, ...)  do {if (CONFIG_LOG_LEVEL >= LOG_ERROR)   LOG_PRINTF(LOG_FORMAT(E, format), read_cycle(), tag, ##__VA_ARGS__); } while (0)
#define LOGW(tag, format, ...)  do {if (CONFIG_LOG_LEVEL >= LOG_WARN)    LOG_PRINTF(LOG_FORMAT(W, format), read_cycle(), tag, ##__VA_ARGS__); } while (0)
#define LOGI(tag, format, ...)  do {if (CONFIG_LOG_LEVEL >= LOG_INFO)    LOG_PRINTF(LOG_FORMAT(I, format), read_cycle(), tag, ##__VA_ARGS__); } while (0)
#define LOGD(tag, format, ...)  do {if (CONFIG_LOG_LEVEL >= LOG_DEBUG)   LOG_PRINTF(LOG_FORMAT(D, format), read_cycle(), tag, ##__VA_ARGS__); } while (0)
#define LOGV(tag, format, ...)  do {if (CONFIG_LOG_LEVEL >= LOG_VERBOSE) LOG_PRINTF(LOG_FORMAT(V, format), read_cycle(), tag, ##__VA_ARGS__); } while (0)
#else
#define LOGE(tag, format, ...)
#define LOGW(tag, format, ...)
#define LOGI(tag, format, ...)
#define LOGD(tag, format, ...)
#define LOGV(tag, format, ...)
#endif  /* LOG_ENABLE */

#ifdef __cplusplus
}
#endif


#endif /* _SYSLOG_H */
