from threading import Event, Thread
from typing import Callable


class Alarm:
    def __init__(self, seconds: float, callback: Callable[[], None]):
        self._seconds = seconds
        self._callback = callback
        self._stop = Event()
        self._thread = Thread(target=self._loop)

    def _loop(self):
        while not self._stop.wait(timeout=self._seconds):
            self._callback()

    def start(self):
        self._stop.clear()
        self._thread.start()

    def stop(self):
        self._stop.set()
        self._thread.join()

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, *_):
        self.stop()
