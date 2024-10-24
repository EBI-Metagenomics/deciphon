import time

from loguru import logger


class ProgressLogger:
    def __init__(self, desc: str):
        self._percent: int = 0
        self._desc = desc
        self._last_time = 0.0

    def start(self):
        self._last_time = time.monotonic() - 10.0

    def stop(self):
        self._print()
        if self._percent == 100:
            logger.info(f"{self._desc}: done!")

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, *_):
        self.stop()

    def _print(self):
        logger.info(f"{self._desc}: {self.percent}% completed")

    @property
    def percent(self):
        return self._percent

    @percent.setter
    def percent(self, x: int):
        self._percent = x
        elapsed = time.monotonic() - self._last_time
        if elapsed > 5:
            self._print()
            self._last_time += elapsed
