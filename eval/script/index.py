__all__ = ["Index"]


class Index:
    @property
    def count(self) -> int:
        ...

    def index(self, _: str) -> int:
        ...

    def keys(self) -> int:
        ...
