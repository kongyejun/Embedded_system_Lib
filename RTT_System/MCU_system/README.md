## 简介
1.SEGGER文件夹是官方SEGGER工程,本工程为了节约传输成本,所以自己实现了数据格式化输出log_vsnprintf函数
2.log_system文件夹是MCU侧日志实现,文件结构为:
    log_system
    |-inc               工程头文件
    |-test              一些其他作用的代码d
    |-log_protocol.c    协议层(格式化层)
    |-log_system.c      用户层(用户API接口)
3.log_vsnprintf的主要思想为,将%c/%s等字符型操作直接将可变参数嵌入到发送字符串中,而%d/%x/%f等数值型参数则不进行解析成ASCII字符而是直接跟随在格式化符后面输出
    如: ("System %s time:%d\n", "init",32) --转换后的数据--> ("System init time:%d[int32_t字节的原始数据]\n\0")
