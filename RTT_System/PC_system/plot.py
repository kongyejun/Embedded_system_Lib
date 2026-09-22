import threading
from collections import deque
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# 让中文正常显示（Windows）
plt.rcParams['font.sans-serif'] = ['SimHei']    # 黑体
plt.rcParams['axes.unicode_minus'] = False       # 负号正常显示


class RTTPlotter:
    """实时多通道曲线绘制
    只负责接收数据点并渲染，不做任何解析。
    数据线程调用 push()，绘图跑在主线程 show()。
    """

    def __init__(self, channels=(0, 1, 2, 3), maxlen=2000, interval=50):
        self.maxlen = maxlen
        self.interval = interval
        self._data = {ch: {"x": deque(maxlen=maxlen), "y": deque(maxlen=maxlen)}
                      for ch in channels}
        self._count = {ch: 0 for ch in channels}    # 本地递增序号
        self._lock = threading.Lock()

        self._fig, self._ax = plt.subplots()
        self._lines = {}
        for ch in channels:
            (line,) = self._ax.plot([], [], label=f"CH{ch}")
            self._lines[ch] = line
        self._ax.set_xlabel("样点数")
        self._ax.set_ylabel("原始值 (RAW)")
        self._ax.set_title("RTT ADC 实时波形")
        self._ax.legend(loc="upper right")
        self._ax.grid(True)

    def push(self, ch, raw):
        """数据线程入口：塞入一个采样点。未知通道自动忽略。
        seq 由外部解析后传入，此处仅使用本地递增序号作为 X 轴。
        """
        buf = self._data.get(ch)
        if buf is None:
            return
        with self._lock:
            buf["x"].append(self._count[ch])    # 用本地递增值，永不回绕
            buf["y"].append(raw)
            self._count[ch] += 1

    def _update(self, _frame):
        with self._lock:
            for ch, line in self._lines.items():
                buf = self._data[ch]
                line.set_data(buf["x"], buf["y"])
        # 自动调整坐标范围
        self._ax.relim()
        self._ax.autoscale_view()
        return list(self._lines.values())

    def show(self):
        """主线程调用，阻塞直到窗口关闭"""
        self._ani = animation.FuncAnimation(
            self._fig, self._update,
            interval=self.interval, blit=False, cache_frame_data=False
        )
        plt.show()