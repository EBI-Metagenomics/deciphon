__all__ = ["Index"]


class Index:
    @property
    def count(self) -> int:
        ...

    def index(self, x: str) -> int:
        ...

    def keys(self) -> int:
        ...
