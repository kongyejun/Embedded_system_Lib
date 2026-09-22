import re
import queue
import threading

class RTTAnalyzer:
    def __init__(self, rules):
        # name -> compiled pattern，顺序即优先级
        self._rules = [(name, re.compile(pat)) for name, pat in rules]
        self.queues = {name: queue.Queue() for name, _ in rules}
        self.queues["unmatched"] = queue.Queue()
        self._lock = threading.Lock()
        self._line_buf = ""

    def feed(self, text):
        """下层回调入口：字节流按 \n 重新组行"""
        with self._lock:
            self._line_buf += text
            *lines, self._line_buf = self._line_buf.split("\n")
        for line in lines:
            line = line.rstrip("\r")     # 兼容 \r\n
            if line:
                self._classify(line)

    def _classify(self, line):
        for name, pat in self._rules:
            m = pat.match(line)
            if m:
                # 队列里放 (原始行, 解析出的字段dict)
                self.queues[name].put((line, m.groupdict()))
                return
        self.queues["unmatched"].put((line, {}))

    def get(self, name, timeout=None):
        try:
            return self.queues[name].get(timeout=timeout)
        except queue.Empty:
            return None