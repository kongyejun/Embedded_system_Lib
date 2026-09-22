from core.rtt_connet import RTTReader
from core.rtt_analysis import RTTAnalyzer
from plot import RTTPlotter
from log import RTTLogger
import threading
import time
#------------------ 
#    队列消费区
#------------------ 
def feed_plot():
    while True:
        item = analyzer.get("adc_sample", timeout=1)
        if item:
            _, f = item
            plotter.push(0,int(f["raw0"])/8388607*5)
            plotter.push(1,int(f["raw1"])/8388607*5)
            plotter.push(2,int(f["raw2"])/8388607*5)
            plotter.push(3,int(f["raw3"])/8388607*5)

def feed_log():
    while True:
        item = analyzer.get("log", timeout=1)
        if item:
            _, f = item
            logger.push(f["level"], f["func"], f["msg"])
#------------------ 
#       main区
#------------------ 

rules = [
  # (name,       正则,                                  优先级=顺序)
    ("adc_sample", r"^RE(?P<seq>\d+),(?P<raw0>\d+),(?P<raw1>\d+),(?P<raw2>\d+),(?P<raw3>\d+)$"),
    ("adc_limit",  r"^ADC_LIMIT:(?P<vals>[\d,]+)$"),
    ("log",        r"^(?P<level>[IWED])(?P<func>\w+):(?P<msg>.*)$"),
]
# 核心与匹配规则开启
analyzer = RTTAnalyzer(rules)
reader   = RTTReader(chip_name="STM32H750VB", on_data=analyzer.feed, log_file=True)
reader.start()

# 日志器
logger   = RTTLogger(to_file="RTT_DEBUG_SYSTEM/log/rtt.log")
threading.Thread(target=feed_log,  daemon=True).start()

# # 绘图器
plotter  = RTTPlotter(channels=(0, 1, 2, 3), maxlen=2000)
# threading.Thread(target=feed_plot, daemon=True).start()
# plotter.show()

# 等待关闭
try:
    while True:
        time.sleep(1)          # 或 signal.pause()（仅 Unix）
except KeyboardInterrupt:
    pass
finally:
    reader.stop()
    logger.close()