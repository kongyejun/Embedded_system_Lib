from dataclasses import dataclass
from datetime import datetime
from typing import Callable, Optional


@dataclass(frozen=True)
class LogLevel:
    DEBUG: str = "DEBUG"
    INFO: str = "INFO"
    WARN: str = "WARN"
    ERROR: str = "ERROR"
    SUCCESS: str = "SUCCESS"


class AppLogger:
    """统一日志接口：log(level, message)"""

    TIME_FMT = "%H:%M:%S"
    LEVEL_WIDTH = 11
    LEVEL_ALIGN = "^"
    PAD_LEVEL_SPACE = False

    def __init__(self, sink: Optional[Callable[[str, str], None]] = None):
        self.sink = sink or self._default_sink

    def log(self, level: str, message: str):
        self.sink(level, message)

    def debug(self, message: str):
        self.log(LogLevel.DEBUG, message)

    def info(self, message: str):
        self.log(LogLevel.INFO, message)

    def warn(self, message: str):
        self.log(LogLevel.WARN, message)

    def error(self, message: str):
        self.log(LogLevel.ERROR, message)

    def success(self, message: str):
        self.log(LogLevel.SUCCESS, message)

    @classmethod
    def format_line(cls, level: str, message: str) -> str:
        ts = datetime.now().strftime(cls.TIME_FMT)

        if cls.PAD_LEVEL_SPACE:
            level_text = f"{level:{cls.LEVEL_ALIGN}{cls.LEVEL_WIDTH}}"
        else:
            level_text = level if len(level) >= cls.LEVEL_WIDTH else f"{level:{cls.LEVEL_ALIGN}{cls.LEVEL_WIDTH}}"

        return f"[{ts}] [{level_text}]\t{message}"

    @staticmethod
    def _default_sink(level: str, message: str):
        print(AppLogger.format_line(level, message))