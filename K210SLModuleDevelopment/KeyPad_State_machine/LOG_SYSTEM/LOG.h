#ifndef __LOG_H_
#define __LOG_H_

#define OPEN_LOG
#define LOG_LEVEL LOG_INFO
//使传入的形参中包含调用 函数名 和 行号
#define EMLOG(level, fmt, ...) __EM_LOG(level, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

// // 重置所有颜色和样式
// #define COLOR_RESET    "\033[0m"    // 重置所有属性

// // 基本前景色（文本颜色）
// #define COLOR_BLACK    "\033[30m"   // 黑色
// #define COLOR_RED      "\033[31m"   // 红色
// #define COLOR_GREEN    "\033[32m"   // 绿色
// #define COLOR_YELLOW   "\033[33m"   // 黄色
// #define COLOR_BLUE     "\033[34m"   // 蓝色
// #define COLOR_MAGENTA  "\033[35m"   // 紫色（洋红色）
// #define COLOR_CYAN     "\033[36m"   // 青色
// #define COLOR_WHITE    "\033[37m"   // 白色

//定义日志等级
typedef enum{
    LOG_DEBUG=0,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
}E_LOG_LEVE;

void __EM_LOG(const int LEVEL,const char* fun,const int line,const char* fmt,...);
void printf_green(const char *s);
void printf_red(const char *s);
void printf_yellow(const char *s);
void printf_blue(const char *s);
void printf_pink(const char *s);
void printf_cyan(const char *s);
#endif
