import sys
from typing import TypeVar

if sys.version_info >= (3, 13):
    from queue import Queue, ShutDown
else:
    from deciphon_worker.queue313 import Queue, ShutDown

T = TypeVar("T")


def queue_loop(x: Queue[T]):
    while True:
        try:
            yield x.get()
            x.task_done()
        except ShutDown:
            break
