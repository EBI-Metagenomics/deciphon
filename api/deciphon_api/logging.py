import logging
import sys
from enum import Enum
from types import FrameType
from typing import cast

from loguru import logger

__all__ = ["LoggingLevel", "RepeatMessageHandler"]


class RepeatMessageHandler:
    def __init__(self, target=sys.stderr):
        self._target = target
        self._previous_args = None
        self._repeats = 0

    def write(self, message):
        args = (message.record["message"], message.record["level"].no)
        if self._previous_args == args:
            self._repeats += 1
            return
        if self._repeats > 0:
            self._target.write(f"[Previous message repeats ×{self._repeats}]\n")
        self._target.write(message)
        self._repeats = 0
        self._previous_args = args


class LoggingLevel(str, Enum):
    CRITICAL = "critical"
    ERROR = "error"
    WARNING = "warning"
    INFO = "info"
    DEBUG = "debug"
    NOTSET = "notset"

    @property
    def level(self) -> int:
        return logging.getLevelName(self.name)


class InterceptHandler(logging.Handler):
    def emit(self, record: logging.LogRecord) -> None:  # pragma: no cover
        # Get corresponding Loguru level if it exists
        try:
            level = logger.level(record.levelname).name
        except ValueError:
            level = str(record.levelno)

        # Find caller from where originated the logged message
        frame, depth = logging.currentframe(), 2
        while frame.f_code.co_filename == logging.__file__:  # noqa: WPS609
            frame = cast(FrameType, frame.f_back)
            depth += 1

        logger.opt(depth=depth, exception=record.exc_info).log(
            level,
            record.getMessage(),
        )
