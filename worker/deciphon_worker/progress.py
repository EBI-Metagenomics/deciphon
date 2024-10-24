from threading import Event, Thread, Semaphore
from typing import Callable


class Progress:
    def __init__(self, callback: Callable[[int], None]):
        self._callback = callback
        self._percent = 0
        self._stop = False
        self._event = Event()
        self._semaphore = Semaphore()
        self._thread = Thread(target=self._loop)

    def _loop(self):
        last_percent = self.percent

        while True:
            self._event.wait()

            if self._stop:
                break

            if self.percent != last_percent:
                last_percent = self.percent

    def start(self):
        self._percent = 0
        self._stop = False
        self._event.clear()
        self._thread.start()

    def stop(self):
        self._stop = True
        self._event.set()
        self._thread.join()

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, *_):
        self.stop()

    @property
    def percent(self):
        return self._percent

    @percent.setter
    def percent(self, x: int):
        if x != self._percent:
            self._percent = x
            self._event.set()
