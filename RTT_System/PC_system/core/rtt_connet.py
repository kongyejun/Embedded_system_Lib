import pylink
import time
import struct
import threading
from datetime import datetime
import os
class RTTReader:
    KONWN_RTT_ADDR      = 0x20001630    # 已知的 RTT 控制块地址,用于加速连接
    FRAME_HEAD          = 0xAA          # 帧起始标志字节
    FRAME_MIN_LEN       = 3             # 帧最短长度: 起始位+长度位+校验位
    FLUSH_INTERVAL      = 0.5           # 每0.5秒刷新一次文件
    FRAME_ERROR_COUNT   = 0             # 帧校验失败/丢帧计数,用于排查链路质量
    """
            初始化 RTT 读取器
    RTT 数据读取器，支持二进制帧解析和自动重连
    """
    def __init__(
        self,
        chip_name="STM32F407VE",    # 目标芯片名称
        target_ch=0,                #  RTT 上行通道号
        once_read_size=2048,        # 单次读取最大字节数
        log_file=True,              # 日志文件路径,True 使用默认路径,False/None 禁用
        read_interval=0.001,        # 读取间隔（秒）
        on_data=None,               # 数据回调函数 on_data(text: str)
    ):
        # 1.资源初始化
        self.chip_name = chip_name
        self.target_ch = target_ch
        self.once_read_size = once_read_size
        self.read_interval = read_interval
        self.on_data = on_data
        self._jlink = None                          # JLink 实例句柄
        self._stop_event = threading.Event()        # 线程停止信号
        self._connect_thread = None                 # 连接线程句柄
        # 2.日志文件创建
        SCRIPT_DIR = os.path.join(
            os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
            'raw_txt'
        )
        os.makedirs(SCRIPT_DIR, exist_ok=True)   # 目录不存在自动创建
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")   # 时间戳,如 20240115_143025
        if log_file is False:
            self.log_file_path = None
        else:
            self.log_file_path = os.path.join(SCRIPT_DIR, f"log_{timestamp}.txt")

    def start(self):
        """非阻塞启动，后台线程连接 RTT 并持续读取"""
        if self._connect_thread and self._connect_thread.is_alive():
            raise RuntimeError("RTTReader 已在运行")
        self._stop_event.clear()
        self._connect_thread = threading.Thread(target=self._connect_loop, daemon=True)
        self._connect_thread.start()

    def stop(self):
        """停止所有线程并释放资源"""
        self._stop_event.set()
        if self._connect_thread:
            self._connect_thread.join(timeout=5)
            self._connect_thread = None

    @property
    def is_running(self):
        """返回 RTT 读取器是否正在运行"""
        return (
            self._connect_thread is not None
            and self._connect_thread.is_alive()
        )

    def _connect_loop(self):
        """连接线程：状态机驱动，断线/无供电自动重连"""
        state = "DISCONNECTED"
        power_warned = False          # ← 断电提示是否已打印过
        while not self._stop_event.is_set():
            try:
                if state == "DISCONNECTED":
                    self._jlink = pylink.JLink()
                    self._jlink.open()
                    self._jlink.connect(chip_name=self.chip_name, speed="auto", verbose=True)
                    self._jlink.set_reset_strategy(pylink.JLinkResetStrategyCortexM3.RESETPIN)
                    power_warned = False          # ← 连上了，重置，下次断电重新提示
                    print(f"设备: ({self._jlink.core_name()}) {self.chip_name}")
                    print(f"速度: {self._jlink.cpu_speed(silent=True) / 1e6:.2f} MHz/ {self._jlink.speed} kHz")
                    state = "CONNECTED"
                elif state == "CONNECTED":
                    print("等待 RTT 控制块...")
                    self._jlink.rtt_start()
                    time.sleep(1)
                    if self._precise_addr_search() or self._auto_addr_search():
                        print("RTT 就绪，开始读取")
                        state = "STREAMING"
                    else:
                        print("[!] 未找到 RTT 控制块，设备可能未上电")
                        state = self._cleanup("DISCONNECTED")
                elif state == "STREAMING":
                    self._read_loop()
                    if not self._stop_event.is_set():
                        print("读取退出，准备重连...")
                    state = self._cleanup("DISCONNECTED")
                elif state == "WAIT_POWER":
                    self._stop_event.wait(timeout=2)
                    state = "DISCONNECTED"
            except Exception as e:
                msg = str(e)
                if any(h in msg.lower() for h in ("no power", "no cpu", "could not connect")):
                    if not power_warned:          # ← 只打印一次
                        print(f"[等待设备上电] {msg}")
                        power_warned = True
                    state = self._cleanup("WAIT_POWER")
                else:
                    print(f"连接异常: {e}")
                    state = self._cleanup("DISCONNECTED")

    def _cleanup(self, next_state):
        """统一清理并返回下一个状态"""
        self._safe_rtt_stop()
        if self._jlink is not None:
            try:
                self._jlink.close()
            except Exception:
                pass
            self._jlink = None
        return next_state
    
    def _precise_addr_search(self):
        """精确地址搜索，直接指定已知的 RTT 控制块地址"""
        print(f"[*] 阶段1: 直接指定 RTT 地址 0x{self.KONWN_RTT_ADDR:08X}")
        self._jlink.exec_command(f"SetRTTAddr 0x{self.KONWN_RTT_ADDR:08X}")
        self._jlink.rtt_start()
        if self._wait_rtt_ready(timeout=3.0):
            print("[+] RTT 连接成功 (精确地址)")
            return True
        print("[!] 精确地址搜索失败")
        self._jlink.rtt_stop()
        self._jlink.exec_command("SetRTTAddr 0")
        return False
    
    def _auto_addr_search(self):
        """自动地址搜索，在整个 RAM 区域搜索 RTT 控制块"""
        print(f"[*] 阶段2: 自动搜索阶段")
        self._jlink.rtt_start()
        if self._wait_rtt_ready(timeout=40.0):
            print(f"[+] RTT 连接成功 (自动地址)")
            print(f"[!] 提示: CB 地址已变化，请在 .map 中搜索 '_SEGGER_RTT'")
            print(f"[!] 更新 KNOWN_RTT_ADDR 可加速下次连接")
            return True
        print("[!] 自动地址搜索失败")
        self._jlink.rtt_stop()
        return False
    
    def _wait_rtt_ready(self, timeout=5.0):
        """轮询等待 RTT 就绪"""
        start = time.time()                         # 轮询起始时间,用于超时判断
        while time.time() - start < timeout:
            try:
                num_up = self._jlink.rtt_get_num_up_buffers()   # 上行缓冲区数量
                if num_up > 0:
                    print(f"RTT 已连接!,通道如下:")
                    num_down = self._jlink.rtt_get_num_down_buffers()   # 下行缓冲区数量
                    for ch in range(num_up):
                        desc = self._jlink.rtt_get_buf_descriptor(ch, True)     # 上行缓冲区描述符
                        print(f"  上行[{ch}]: 缓冲区名称={desc.name}, 缓冲区大小={desc.SizeOfBuffer}")
                    for ch in range(num_down):
                        desc = self._jlink.rtt_get_buf_descriptor(ch, False)    # 下行缓冲区描述符
                        print(f"  下行[{ch}]: 缓冲区名称={desc.name}, 缓冲区大小={desc.SizeOfBuffer}")
                    return True
            except pylink.JLinkRTTException:
                pass
            time.sleep(0.2)
        return False
    
    def _extract_frames(self, buf):
        """从字节流 buf 中切出所有完整帧，逐个 yield payload;就地消费 buf
        帧结构: [帧头][payload长度N][校验][payload N字节]
        buf[1] = 用户数据长度N(不含头/长度/校验), 整帧长度 = N + 3
        校验 = payload N 个字节的异或"""
        while len(buf) >= self.FRAME_MIN_LEN:                   # 至少要有 头+长度+校验 三字节
            head = buf.find(self.FRAME_HEAD)                    # 帧头 0xAA 在 buf 中的位置
            if head < 0:                                        # 整段无帧头,全部丢弃
                buf.clear() 
                break   
            del buf[:head]                                      # 丢弃帧头前的垃圾字节
            if len(buf) < self.FRAME_MIN_LEN:                   # 头/长度/校验未到齐,等下一批
                break   
            payload_len = buf[1]                                # 用户数据长度N(不含头/长度/校验)
            frame_len = payload_len + self.FRAME_MIN_LEN        # 整帧长度 = 头+长度+校验+payload
            if len(buf) < frame_len:                            # 半包,等后续数据补齐
                break   
            checksum = 0                                        # 计算出的 XOR 校验值
            for b in buf[self.FRAME_MIN_LEN:frame_len]:         # b: payload 每个字节
                checksum ^= b   
            if checksum != buf[2]:                              # 校验字节在索引2
                del buf[0]                                      # 只丢1字节重新同步
                self.FRAME_ERROR_COUNT += 1
                continue
            payload = bytes(buf[self.FRAME_MIN_LEN:frame_len])  # 有效负载(payload N字节)
            del buf[:frame_len]                                 # 消费整帧
            yield payload

    def _flush_to_file(self, f, text_buffer, last_flush_time):
        """按时间间隔或缓冲量批量写盘，返回最新的刷盘时间戳"""
        current_time = time.time()                  # 当前时间戳,用于判断是否需要刷盘
        if f and text_buffer and (current_time - last_flush_time >= self.FLUSH_INTERVAL or len(text_buffer) >= 100):
            f.write("".join(text_buffer))
            f.flush()
            text_buffer.clear()
            return current_time
        return last_flush_time
    
    def _dispatch_text(self, text, text_buffer):
        """将解码文本入队缓存，并安全触发外部回调"""
        text_buffer.append(text)
        if self.on_data:
            try:
                self.on_data(text)
            except Exception as e:
                print(f"on_data 回调异常: {e}")     # 防止外部回调异常打断读取线程

    def _read_loop(self):
        """读取任务,只负责调度: 读取 -> 拆帧 -> 分发 -> 刷盘"""
        try:
            f = open(self.log_file_path, "a", encoding="utf-8", buffering=8192) if self.log_file_path else None  # 日志文件句柄,未配置路径时为 None
            try:
                if f:
                    f.write(f"\n--- {datetime.now()} 开始记录 ---\n")
                buf = bytearray()                   # 累积未解析完的字节流
                text_buffer = []                    # 待批量写入文件的文本片段
                last_flush_time = time.time()       # 上次刷盘时间戳
                empty_read_count = 0                # 连续空读计数,用于降频休眠
                while not self._stop_event.is_set():
                    try:
                        data = self._jlink.rtt_read(self.target_ch, self.once_read_size)  # 本次读到的原始字节
                    except Exception:
                        print("读取异常, 准备重连...")
                        return
                    if not data:                    # 空读:累计到阈值才休眠,降低空转
                        empty_read_count += 1
                        if empty_read_count >= 3:
                            time.sleep(self.read_interval)
                        continue
                    empty_read_count = 0
                    buf.extend(data)
                    for payload in self._extract_frames(buf):   # payload: 一帧有效负载
                        self._dispatch_text(self._decode_payload(payload), text_buffer)
                    last_flush_time = self._flush_to_file(f, text_buffer, last_flush_time)
                if f and text_buffer:               # 退出前写入剩余数据
                    f.write("".join(text_buffer))
                    f.flush()
            finally:
                if f:
                    f.close()
        except Exception as e:
            print(f"线程异常: {e}")

    def _parse_flags(self, payload: bytes, start: int) -> int:
        """解析标志字符串，返回标志字符串结束位置"""
        pos = start                                 # 当前扫描位置,逐步跳过 标志/宽度/精度
        while pos < len(payload) and chr(payload[pos]) in '-0+# ':   # 跳过标志字符(-, 0, +, #, 空格)
            pos += 1
        while pos < len(payload) and ord('0') <= payload[pos] <= ord('9'):  # 跳过宽度数字
            pos += 1
        if pos < len(payload) and payload[pos] == ord('.'):         # 跳过精度(. 后跟数字)
            pos += 1
            while pos < len(payload) and ord('0') <= payload[pos] <= ord('9'):
                pos += 1
        return pos
    
    def _format_int(self, val: int, spec: str, flags: str) -> str:
        """格式化整数"""
        if spec == 'p':                             # 指针类型:按十六进制补零输出
            width = 8                               # 指针默认宽度
            i = 0                                    # flags 中数字部分的起始位置
            while i < len(flags) and not flags[i].isdigit():
                i += 1
            if i < len(flags):
                width = int(flags[i:].split('.')[0] or '8')
            return f"0x{val:0{width}X}"
        try:
            fmt_str = f"%{flags}{spec}"             # 拼出标准 printf 格式串复用 % 运算符
            return fmt_str % val
        except Exception:
            if spec in 'xX':
                return f"{val:x}" if spec == 'x' else f"{val:X}"
            return str(val)
        
    def _format_float(self, val: float, flags: str) -> str:
        """格式化浮点数"""
        try:
            fmt_str = f"%{flags}f"                  # 拼出标准 printf 格式串复用 % 运算符
            return fmt_str % val
        except Exception:
            return f"{val:.6f}"
        
    def _decode_payload(self, payload: bytes) -> str:
        """解析二进制编码的 payload,还原为可读字符串"""
        result = []
        n = len(payload)
        i = 0
        while i < n:
            pct = payload.find(0x25, i)             # 下一个 '%'
            if pct < 0:
                result.append(payload[i:].decode('utf-8', errors='replace'))
                break
            if pct > i:
                result.append(payload[i:pct].decode('utf-8', errors='replace'))
            i = pct
            if i + 1 >= n:                          # '%' 是末字节
                break
            ## %% 转义
            if payload[i + 1] == 0x25:
                result.append('%')
                i += 2
                continue
            # ① 先解析标志/宽度/精度(从 '%' 之后开始)
            flag_start = i + 1
            flag_end = self._parse_flags(payload, flag_start)
            if flag_end >= n:                       # 没有类型符了
                result.append(payload[i:flag_end].decode('ascii', errors='replace'))
                break
            # ② flag_end 处才是类型符
            spec = payload[flag_end]
            flags = payload[flag_start:flag_end].decode('ascii', errors='ignore')
            data_start = flag_end + 1               # ③ 类型符之后才是二进制数据
            if spec in (ord('d'), ord('i'), ord('u'), ord('x'), ord('X'), ord('p')):
                if data_start + 4 > n:
                    break
                signed = spec in (ord('d'), ord('i'))
                val = int.from_bytes(payload[data_start:data_start + 4], 'big', signed=signed)
                result.append(self._format_int(val, chr(spec), flags))
                i = data_start + 4
            elif spec == ord('f'):
                if data_start + 8 > n:
                    break
                dval = struct.unpack('>d', payload[data_start:data_start + 8])[0]
                result.append(self._format_float(dval, flags))
                i = data_start + 8
            elif spec == ord('c'):                  # 固件里 %c 存 1 字节
                if data_start + 1 > n:
                    break
                result.append(chr(payload[data_start]))
                i = data_start + 1
            elif spec == ord('s'):                  # 固件里 %s 存到 '\0' 为止
                end = payload.find(0x00, data_start)
                if end < 0:
                    end = n
                result.append(payload[data_start:end].decode('utf-8', errors='replace'))
                i = end + 1
            else:                                   # 未识别类型符,原样输出
                result.append(payload[i:flag_end + 1].decode('ascii', errors='replace'))
                i = flag_end + 1

        return "".join(result)
    
    def _safe_rtt_stop(self):
        """安全停止 RTT,忽略异常"""
        try:
            if self._jlink:
                self._jlink.rtt_stop()
        except Exception as e:
            print(f"暂停异常: {e}")

if __name__ == "__main__":
    reader = RTTReader(
        chip_name="STM32H750VB",
        log_file = True,
    )
    reader.start()
    while True:
        reader.is_running
        time.sleep(10)