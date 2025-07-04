#include "LOG_SYSTEM\LOG.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static char* buff;//全局变量
static va_list arg;//可变参数列表
static int8_t is_print = 1; //是否可以打印日志

void LOG_Stop(void){
    is_print = 0;
}
void LOG_Start(void){
    is_print = 1;
}

//红色
void printf_red(const char *s){
    printf("\033[0m\033[1;31m%s\033[0m", s);
}
//绿色
void printf_green(const char *s){
    printf("\033[0m\033[1;32m%s\033[0m", s);
}
//黄色
void printf_yellow(const char *s){
    printf("\033[0m\033[1;33m%s\033[0m", s);
}
//蓝色
void printf_blue(const char *s){
    printf("\033[0m\033[1;34m%s\033[0m", s);
}
//粉色
void printf_pink(const char *s){
    printf("\033[0m\033[1;35m%s\033[0m", s);
}
// 青色
void printf_cyan(const char *s){
    printf("\033[0m\033[1;36m%s\033[0m", s);
}

const char* EM_LOGLevelGet(const int level)
{
    switch(level){
        case 0:return "DEBUG";
        case 1:return "\033[32mINFO\033[0m";//\033[颜色代码m  -----表示切换之后输出的颜色
        case 2:return "\033[35mWARN\033[0m";
        case 3:return "\033[31mERROR\033[0m";
    }
    return "UNKNOW";
}

//不定参数的使用
void __EM_LOG(const int LEVEL,const char* fun,const int line,const char* fmt,...){
    //判断是否开启日志
    if(is_print){
        //判断带输出日志等级是否达标
        if(LEVEL>=LOG_LEVEL){
            va_start(arg,fmt);//这里fmt的表示形参的最后一个固定参数
            //为什么要这一步,原因是为了让不定参数中的数据可以 自动匹配类型 并以 字符串 的形式存起来
            int len = vsnprintf(NULL,0,fmt,arg) + 1;//加1是因为返回不算'\0'
                //vsnprintf()是指:
                //将 可变参数arg 按照 fmt(字符串)中指定的类型 写入到 参数1 所指的位置, 并限定只写 n 次
                //第n次为传入'\0' 作为字符串结束标志
                //返回 待写入的字符个数(比如%f 占8个字符(未计算整数),%.2f占4个),计数时注意不包括'\0'
            buff = (char*)malloc(sizeof(char)*len);
            if (buff == NULL) {
                printf("LOG:Memory allocation failed\n");
                return;
            }
            vsprintf(buff,fmt,arg);//将数据写入buff中
            buff[len]='\0';
            va_end(arg);
            printf("[%s] [\033[0m\033[1;33m%s\033[0m \033[0m\033[1;32m%d\033[0m] %s",EM_LOGLevelGet(LEVEL),fun,line,buff);
            //输出 日志等级 信息位置 信息
            free(buff);
            buff = NULL;
        }
        return ;
    }
}
