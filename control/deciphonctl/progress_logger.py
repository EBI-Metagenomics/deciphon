import time

from loguru import logger


class ProgressLogger:
    def __init__(self, total: int, desc: str):
        self._total = total
        self._consumed = 0
        self._desc = desc
        self._last_time = 0.0

    def __enter__(self):
        self._last_time = time.monotonic() - 10.0
        return self

    def __exit__(self, *_):
        self._print()
        if self._consumed == self._total:
            logger.info(f"{self._desc}: done!")

    def _print(self):
        logger.info(f"{self._desc}: {self.percent:06.2f}% completed")

    @property
    def total(self):
        return self._total

    @property
    def consumed(self):
        return self._consumed

    @property
    def percent(self):
        if self.total == 0:
            return 100.0
        return 100.0 * (self._consumed / self._total)

    def consume(self):
        self._consumed += 1
        elapsed = time.monotonic() - self._last_time
        if elapsed > 5:
            self._print()
            self._last_time += elapsed
