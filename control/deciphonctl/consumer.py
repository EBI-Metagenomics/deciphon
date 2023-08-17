import time
from abc import ABC, abstractmethod
from multiprocessing import JoinableQueue
from typing import cast

from loguru import logger

from deciphonctl.signals import ignore_sigint


class Consumer(ABC):
    def __init__(self, queue: JoinableQueue):
        self._queue = queue

    @abstractmethod
    def callback(self, message: str):
        ...

    def run(self):
        ignore_sigint()
        while True:
            try:
                message = cast(str, self._queue.get())
                self.callback(message)
                self._queue.task_done()
            except KeyboardInterrupt:
                assert False
            except Exception as exception:
                logger.exception(exception)
                time.sleep(1)
