__all__ = ["Percent"]


class Percent:
    def __init__(self, total: int):
        assert total >= 0
        self._total = total
        self._consumed = 0
        self._last_percent = 0

    def consume(self, x: int) -> int:
        self._consumed += x
        assert self._consumed <= self._total
        inc = self.percent - self._last_percent
        if inc > 0:
            self._last_percent = self.percent
        return inc

    @property
    def percent(self) -> int:
        return self._consumed // self._total
