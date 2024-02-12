from __future__ import annotations

import signal
from signal import SIGINT, SIGTERM, Signals, SIG_DFL
from typing import List

__all__ = ["default_signalling"]


class default_signalling:
    def __init__(self, signals: List[Signals] = [SIGTERM, SIGINT]):
        self._signals = signals
        self._handlers: List[signal._HANDLER] = []

    def __enter__(self):
        for x in self._signals:
            self._handlers.append(signal.getsignal(x))
            signal.signal(x, SIG_DFL)
        return self

    def __exit__(self, *_):
        for x, y in zip(self._signals, self._handlers):
            signal.signal(x, y)
