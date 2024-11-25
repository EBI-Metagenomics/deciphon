from __future__ import annotations

import signal
from collections.abc import Callable
from signal import SIGINT, SIGTERM, Signals
from typing import List

import typer

__all__ = ["service_exit"]


class service_exit:
    def __init__(self, signals: List[Signals] = [SIGTERM, SIGINT]):
        self._signals = signals
        self._handlers: List[signal._HANDLER] = []
        self._callback: Callable[[], None] | None = None

    def _handler(self, *_):
        if self._callback is not None:
            self._callback()
        raise typer.Exit(1)

    def setup(self, callback: Callable[[], None]):
        self._callback = callback

    def __enter__(self):
        for x in self._signals:
            self._handlers.append(signal.getsignal(x))
            signal.signal(x, self._handler)
        return self

    def __exit__(self, *_):
        for x, y in zip(self._signals, self._handlers):
            signal.signal(x, y)
