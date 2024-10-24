from queue import Queue
from threading import Thread
from typing import Callable

callback_type = Callable[[], None]


class Background:
    def __init__(self):
        self._queue: Queue[callback_type | None] = Queue()
        self._thread = Thread(target=self.loop)

    def fire(self, callback: callback_type):
        self._queue.put(callback)

    def shutdown(self):
        self._queue.put(None)

    def loop(self):
        while True:
            callback = self._queue.get()
            if callback is None:
                break
            callback()
            self._queue.task_done()

    def start(self):
        assert self._queue.empty()
        self._thread.start()

    def stop(self):
        self.shutdown()
        self._thread.join()

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, *_):
        self.stop()
