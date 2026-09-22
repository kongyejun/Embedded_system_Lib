# RTTReader API 参考文档

## 目录
- [类：RTTReader](#类rttreader)
- [构造函数](#构造函数)
- [公共方法](#公共方法)
- [公共属性](#公共属性)
- [私有方法](#私有方法)
- [回调函数规范](#回调函数规范)
- [异常处理](#异常处理)

---

## 类：RTTReader

RTT 数据读取器，支持二进制帧解析和自动重连。

### 类图

```
RTTReader
├── __init__()          # 初始化
├── start()             # 启动读取
├── stop()              # 停止读取
├── is_running          # 运行状态（属性）
└── [私有方法]          # 内部实现
```

---

## 构造函数

### `__init__(chip_name, target_ch, once_read_size, log_file, read_interval, on_data)`

创建 RTTReader 实例。

#### 参数

| 参数名 | 类型 | 默认值 | 说明 |
|--------|------|--------|------|
| `chip_name` | `str` | `"STM32F407VE"` | 目标芯片名称，需与 J-Link 支持的名称匹配 |
| `target_ch` | `int` | `0` | RTT 上行通道号（0-15） |
| `once_read_size` | `int` | `2048` | 单次读取最大字节数（建议 512-4096） |
| `log_file` | `bool/str/None` | `True` | 日志文件配置，见下方说明 |
| `read_interval` | `float` | `0.001` | 读取间隔秒数（0.0001-0.1） |
| `on_data` | `callable/None` | `None` | 数据回调函数，签名为 `fn(text: str)` |

#### `log_file` 参数详解

| 值 | 行为 | 示例 |
|----|------|------|
| `True` | 使用默认文件名 | `Primitive_data_20240115_143025.txt` |
| `"custom.log"` | 自定义文件名（自动加时间戳） | `custom_20240115_143025.log` |
| `False` 或 `None` | 禁用日志记录 | 无文件生成 |

#### 示例

```python
# 最简配置
reader = RTTReader()

# 完整配置
reader = RTTReader(
    chip_name="STM32H750VB",
    target_ch=0,
    once_read_size=4096,
    log_file="debug_output.txt",
    read_interval=0.0005,
    on_data=lambda text: print(f"[RTT] {text}", end='')
)

# 多通道监控
reader_ch0 = RTTReader(target_ch=0, log_file="channel0.log")
reader_ch1 = RTTReader(target_ch=1, log_file="channel1.log")
```

#### 支持的芯片名称示例

常见 STM32 系列：
- `STM32F103C8`
- `STM32F407VE`
- `STM32H750VB`
- `STM32L476RG`
- `STM32G474RE`

其他厂商：
- `nRF52832_xxAA`
- `nRF52840_xxAA`
- `EFR32MG12P432F1024`

**提示**：可通过 J-Link Commander 输入 `?` 查询支持的芯片列表。

---

## 公共方法

### `start()`

非阻塞启动 RTT 读取器，后台线程自动连接并持续读取数据。

#### 签名
```python
def start(self) -> None
```

#### 返回值
无

#### 异常
- `RuntimeError`：如果 RTTReader 已在运行

#### 行为
1. 创建后台连接线程
2. 打开 J-Link 连接
3. 搜索 RTT 控制块
4. 启动数据读取线程
5. 断线后自动重连

#### 示例

```python
reader = RTTReader()
reader.start()

# 主线程可以继续执行其他任务
print("RTT 已启动，后台运行中...")

# 阻塞等待用户中断
try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    reader.stop()
```

---

### `stop()`

停止所有后台线程并释放 J-Link 资源。

#### 签名
```python
def stop(self) -> None
```

#### 返回值
无

#### 异常
无（内部异常已捕获）

#### 行为
1. 设置停止信号
2. 等待连接线程退出（最多 5 秒）
3. 停止 RTT 通信
4. 关闭 J-Link 连接
5. 关闭日志文件

#### 示例

```python
reader = RTTReader()
reader.start()

# 运行一段时间后停止
time.sleep(60)
reader.stop()
print("已停止")
```

---

## 公共属性

### `is_running`

只读属性，返回 RTT 读取器是否正在运行。

#### 签名
```python
@property
def is_running(self) -> bool
```

#### 返回值
- `True`：后台线程正在运行
- `False`：已停止或未启动

#### 示例

```python
reader = RTTReader()

print(reader.is_running)  # False
reader.start()
print(reader.is_running)  # True

# 监控运行状态
while reader.is_running:
    print("运行中...")
    time.sleep(1)
```

---

## 私有方法

以下方法为内部实现，不应被外部直接调用。

### `_connect_loop()`

连接主循环，负责建立 J-Link 连接并管理自动重连。

**职责**：
- 打开并配置 J-Link
- 精确地址搜索（3 秒超时）
- 自动地址搜索（40 秒超时）
- 启动读取线程
- 断线后自动重连

---

### `_precise_addr_search()`

精确地址搜索，使用 `konwn_rtt_addr` 直接指定 RTT 控制块地址。

**返回值**：
- `True`：成功连接
- `False`：精确地址无效，需回退到自动搜索

---

### `_auto_addr_search()`

自动地址搜索，在整个 RAM 区域扫描 RTT 控制块。

**耗时**：通常 20-40 秒

**返回值**：
- `True`：成功连接
- `False`：未找到 RTT 控制块

---

### `_wait_rtt_ready(timeout)`

轮询等待 RTT 就绪。

**参数**：
- `timeout` (float)：超时秒数

**返回值**：
- `True`：RTT 已就绪
- `False`：超时

---

### `_read_loop()`

读取主循环，负责数据读取、拆帧、解码、回调和日志写入。

**循环流程**：
```
读取原始字节 → 累积到缓冲区 → 拆帧 → 解码载荷 → 触发回调 → 批量写日志
```

---

### `_extract_frames(buf)`

从字节流中提取完整帧。

**参数**：
- `buf` (bytearray)：累积的字节流（会被就地修改）

**返回值**：
- Generator，逐个 yield 有效载荷（bytes）

**帧提取逻辑**：
1. 查找帧头 `0xAA`
2. 读取长度字节
3. 验证长度合法性
4. 等待完整帧到达
5. XOR 校验
6. yield 有效载荷
7. 从 buf 中删除已处理的帧

---

### `_decode_payload(payload)`

解码二进制载荷为可读文本。

**参数**：
- `payload` (bytes)：有效载荷数据

**返回值**：
- `str`：解码后的文本

**支持的格式符**：

| 格式符 | 处理方式 |
|--------|----------|
| `%d`, `%i` | 4 字节有符号整数（大端） |
| `%u` | 4 字节无符号整数（大端） |
| `%x`, `%X` | 4 字节十六进制整数 |
| `%p` | 4 字节指针（格式化为 `0xXXXXXXXX`） |
| `%f` | 8 字节双精度浮点数（大端） |
| `%%` | 转义为单个 `%` |

**宽度和精度支持**：
- `%08X`：补零到 8 位十六进制
- `%.2f`：保留 2 位小数
- `%5d`：右对齐到 5 位

---

### `_parse_flags(payload, start)`

解析格式符的标志、宽度和精度。

**参数**：
- `payload` (bytes)：载荷数据
- `start` (int)：起始解析位置

**返回值**：
- `int`：标志字符串结束位置

**识别的标志**：
- `-`：左对齐
- `0`：补零
- `+`：显示正号
- `#`：备用格式
- ` `（空格）：正数前加空格

---

### `_format_int(val, spec, flags)`

格式化整数。

**参数**：
- `val` (int)：整数值
- `spec` (str)：格式符（`'d'`, `'x'`, `'p'` 等）
- `flags` (str)：标志字符串

**返回值**：
- `str`：格式化后的字符串

---

### `_format_float(val, flags)`

格式化浮点数。

**参数**：
- `val` (float)：浮点数值
- `flags` (str)：标志字符串

**返回值**：
- `str`：格式化后的字符串

---

### `_flush_to_file(f, text_buffer, last_flush_time)`

按时间间隔或缓冲量批量写入日志文件。

**刷盘条件**（满足任一）：
- 距上次刷盘 ≥ 0.5 秒
- 缓冲区累积 ≥ 100 条文本片段

**返回值**：
- `float`：最新的刷盘时间戳

---

### `_dispatch_text(text, text_buffer)`

分发解码后的文本。

**行为**：
1. 将文本加入缓冲区（用于批量写盘）
2. 安全调用 `on_data` 回调（捕获异常）

---

### `_safe_rtt_stop()`

安全停止 RTT 通信，忽略可能的异常。

---

## 回调函数规范

### `on_data` 回调

当解码出新的文本数据时触发。

#### 签名
```python
def on_data(text: str) -> None
```

#### 参数
- `text` (str)：解码后的文本片段（可能包含换行符）

#### 注意事项
1. **执行线程**：在读取线程中执行，非主线程
2. **异常处理**：回调中的异常会被捕获并打印，不会中断读取
3. **性能要求**：应避免耗时操作（如网络请求、大量计算）
4. **线程安全**：如需操作共享资源，注意加锁

#### 示例

```python
# 简单打印
def callback(text):
    print(text, end='')

# 数据过滤
def callback(text):
    if "ERROR" in text:
        print(f"\033[31m{text}\033[0m", end='')  # 红色高亮
    else:
        print(text, end='')

# 数据转发到队列（推荐）
from queue import Queue
data_queue = Queue()

def callback(text):
    data_queue.put(text)

reader = RTTReader(on_data=callback)
reader.start()

# 主线程处理队列数据
while True:
    text = data_queue.get()
    # 处理数据...
```

---

## 异常处理

### 启动时异常

```python
reader = RTTReader()
try:
    reader.start()
except RuntimeError as e:
    print(f"启动失败: {e}")
```

**可能的原因**：
- RTTReader 已在运行（重复调用 `start()`）

### 连接异常

连接异常在后台线程中处理，会自动重连。终端输出示例：

```
连接异常: DLL load failed
读取线程退出, 1 秒后重连...
```

**常见异常**：
- `JLinkException`：J-Link 驱动问题
- `JLinkRTTException`：RTT 通信错误
- `OSError`：USB 连接断开

### 回调异常

回调函数中的异常会被捕获并打印：

```python
def buggy_callback(text):
    raise ValueError("测试异常")

reader = RTTReader(on_data=buggy_callback)
reader.start()

# 终端输出：
# on_data 回调异常: 测试异常
# （读取继续进行，不会中断）
```

---

## 完整使用示例

### 示例 1：基础监控

```python
from rtt_connet import RTTReader
import time

reader = RTTReader(
    chip_name="STM32F407VE",
    log_file=True,
)

reader.start()
print("按 Ctrl+C 停止...")

try:
    while reader.is_running:
        time.sleep(1)
except KeyboardInterrupt:
    reader.stop()
    print("\n已停止")
```

### 示例 2：实时数据分析

```python
import re
from collections import deque

class RTTAnalyzer:
    def __init__(self):
        self.temp_history = deque(maxlen=100)
        
    def on_data(self, text):
        # 提取温度数据
        match = re.search(r'Temp:\s*(-?\d+)', text)
        if match:
            temp = int(match.group(1))
            self.temp_history.append(temp)
            avg = sum(self.temp_history) / len(self.temp_history)
            print(f"当前温度: {temp}°C, 平均: {avg:.1f}°C")

analyzer = RTTAnalyzer()
reader = RTTReader(
    chip_name="STM32F407VE",
    on_data=analyzer.on_data,
    log_file=False,  # 不需要原始日志
)

reader.start()
```

### 示例 3：多通道监控

```python
readers = []

for ch in range(2):
    reader = RTTReader(
        target_ch=ch,
        log_file=f"channel_{ch}.log",
        on_data=lambda text, ch=ch: print(f"[CH{ch}] {text}", end='')
    )
    reader.start()
    readers.append(reader)

# 统一停止
for reader in readers:
    reader.stop()
```

### 示例 4：故障检测

```python
import time

reader = RTTReader(chip_name="STM32F407VE")
reader.start()

# 监控帧错误率
last_error_count = 0
while reader.is_running:
    time.sleep(10)
    current_errors = reader.frame_error_count
    error_rate = (current_errors - last_error_count) / 10
    
    if error_rate > 5:  # 每秒超过 5 个错误帧
        print(f"警告: 帧错误率过高 ({error_rate:.1f}/s)")
    
    last_error_count = current_errors
```

---

## 性能参数调优

### 参数对照表

| 应用场景 | once_read_size | read_interval | log_file |
|---------|----------------|---------------|----------|
| 低速调试 | 512 | 0.01 | True |
| 标准使用 | 2048 | 0.001 | True |
| 高速采集 | 4096 | 0.0001 | False |
| 低功耗目标 | 512 | 0.05 | True |

### 调优建议

1. **提高吞吐量**：增大 `once_read_size`，减小 `read_interval`
2. **降低 CPU 占用**：减小 `once_read_size`，增大 `read_interval`
3. **避免丢帧**：监控 `frame_error_count`，适当调整参数

---

## 版本信息

- **API 版本**：1.0
- **兼容 Python 版本**：3.7+
- **依赖库版本**：pylink-square >= 0.9.0

---

## 另请参阅

- [README.md](README.md) - 使用指南和快速开始
- [rtt_connet.py](rtt_connet.py) - 源代码实现
- [SEGGER RTT 官方文档](https://www.segger.com/products/debug-probes/j-link/technology/about-real-time-transfer/)
