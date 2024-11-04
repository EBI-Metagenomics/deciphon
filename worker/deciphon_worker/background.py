from threading import Thread
from typing import Callable

from deciphon_worker.queue313 import Queue, ShutDown

callback_type = Callable[[], None]


class Background:
    def __init__(self):
        self._queue: Queue[callback_type] = Queue()
        self._thread = Thread(target=self.loop)

    def fire(self, callback: callback_type):
        self._queue.put(callback)

    def loop(self):
        while True:
            try:
                callback = self._queue.get()
            except ShutDown:
                break
            if callback is None:
                break
            callback()
            self._queue.task_done()

    def start(self):
        assert self._queue.empty()
        self._thread.start()

    def stop(self):
        self._queue.shutdown()
        self._thread.join()

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, *_):
        self.stop()
