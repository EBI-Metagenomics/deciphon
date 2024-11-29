import concurrent.futures
from threading import Condition
from typing import TypeVar

T = TypeVar("T")


class Progressor(concurrent.futures.Future[T]):
    def __init__(self):
        super().__init__()
        self._progress = 0
        self._progressing = Condition()
        self._interrupted = False

    def interrupt(self):
        self._interrupted = True

    @property
    def interrupted(self):
        return self._interrupted

    @property
    def progress(self):
        return self._progress

    def set_progress(self, x: int):
        with self._progressing:
            self._progress = x
            self._progressing.notify_all()

    def set_result(self, result):
        with self._progressing:
            super().set_result(result)
            self._progressing.notify_all()

    def set_exception(self, exception):
        with self._progressing:
            super().set_exception(exception)
            self._progressing.notify_all()

    def as_progress(self, timeout: float | None = None):
        last_progress = self._progress
        with self._progressing:
            while not self.done:
                self._progressing.wait(timeout=timeout)
                if self._progress != last_progress:
                    last_progress = self._progress
                    self._condition.release()
                    yield self._progress
                    self._condition.acquire()

        self.result()
