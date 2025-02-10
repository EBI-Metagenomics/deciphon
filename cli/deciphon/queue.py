from multiprocessing import Queue
from typing import Generic, Literal, TypeVar, cast

T = TypeVar("T")
shutdown = Literal["shutdown"]


class ShutDown(Exception):
    """Raised when put/get with shut-down queue."""


class ShuttableQueue(Generic[T]):
    def __init__(self):
        self._queue: Queue[T | shutdown] = Queue()

    def put(self, obj):
        self._queue.put(obj)

    def get(self) -> T:
        item = self._queue.get()
        if item == "shutdown":
            raise ShutDown
        return cast(T, item)

    def shutdown(self):
        self._queue.put("shutdown")


def queue_loop(x: ShuttableQueue[T]):
    while True:
        try:
            yield x.get()
        except ShutDown:
            break
