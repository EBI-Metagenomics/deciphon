# Inspired in the pebble library
from concurrent.futures import Future
from functools import partial
from threading import Thread
from typing import Callable, TypeVar

T = TypeVar("T")


def handler(function: Callable[[], T], future: Future[T]):
    future.set_running_or_notify_cancel()

    try:
        future.set_result(function())
    except Exception as x:
        future.set_exception(x.with_traceback(x.__traceback__))


def launch_thread(function: Callable[[], T], name: str | None = None):
    future: Future[T] = Future()
    thread = Thread(target=partial(handler, function, future), name=name)
    thread.daemon = True
    thread.start()
    return future
