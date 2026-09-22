import sys
import os
import time
import threading


def _enable_win_ansi():
    if os.name != "nt":
        return True
    try:
        import ctypes
        kernel32 = ctypes.windll.kernel32
        h = kernel32.GetStdHandle(-11)
        mode = ctypes.c_uint()
        kernel32.GetConsoleMode(h, ctypes.byref(mode))
        kernel32.SetConsoleMode(h, mode.value | 0x0004)
        return True
    except Exception:
        return False


_LEVEL = {
    "I": ("INFO ", "\033[32m"),   # 绿
    "W": ("WARN ", "\033[93m"),   # 亮黄
    "E": ("ERROR", "\033[91m"),   # 亮红
    "D": ("DEBUG", "\033[90m"),   # 灰
}
_RESET = "\033[0m"
_SEP = "-" * 60          # 分割线


class RTTLogger:
    """日志输出。只负责接收一条日志并渲染，不解析、不碰队列。"""

    def __init__(self, to_file=None, use_color=True):
        # 上色前先确认终端支持，开不了就自动关闭上色
        self.use_color = use_color and _enable_win_ansi()
        self._fp = open(to_file, "a", encoding="utf-8") if to_file else None
        self._lock = threading.Lock()

        # 程序开始：写一条起始分割线（带时间戳，方便区分每次运行）
        if self._fp:
            ts = time.strftime("%Y-%m-%d %H:%M:%S")
            self._fp.write(f"{_SEP} START {ts} {_SEP}\n")
            self._fp.flush()

    def push(self, level, func, msg):
        name, color = _LEVEL.get(level, ("?????", ""))
        ts = time.strftime("%H:%M:%S")
        plain = f"[{ts}] [{name}] {func}: {msg}"

        with self._lock:
            if self.use_color and color:
                sys.stdout.write(f"{color}{plain}{_RESET}\n")
            else:
                sys.stdout.write(plain + "\n")
            sys.stdout.flush()

            if self._fp:
                self._fp.write(plain + "\n")   # 落盘不带颜色
                self._fp.flush()

    def close(self):
        if self._fp:
            self._fp.close()
            self._fp = None      # 防止重复 close 时再写