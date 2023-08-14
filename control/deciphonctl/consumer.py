import time
from abc import ABC, abstractmethod
from multiprocessing import JoinableQueue
from typing import cast

from loguru import logger


class Consumer(ABC):
    def __init__(self, queue: JoinableQueue):
        self._queue = queue

    @abstractmethod
    def callback(self, message: str):
        ...

    def entry_point(self):
        while True:
            try:
                message = cast(str, self._queue.get())
                self.callback(message)
                self._queue.task_done()
            except KeyboardInterrupt:
                logger.info("exiting...")
                return
            except Exception as exception:
                logger.exception(exception)
                time.sleep(1)
