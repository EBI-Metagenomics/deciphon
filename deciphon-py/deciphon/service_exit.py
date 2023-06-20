from __future__ import annotations

import signal
from signal import SIGINT, SIGTERM, Signals
from typing import List

import typer

__all__ = ["service_exit"]


def raise_exit(*_):
    raise typer.Exit(1)


class service_exit:
    def __init__(self, signals: List[Signals] = [SIGTERM, SIGINT]):
        self._signals = signals
        self._handlers: List[signal._HANDLER] = []

    def __enter__(self):
        for x in self._signals:
            self._handlers.append(signal.getsignal(x))
            signal.signal(x, raise_exit)
        return self

    def __exit__(self, *_):
        for x, y in zip(self._signals, self._handlers):
            signal.signal(x, y)
