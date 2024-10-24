from types import TracebackType
from typing import Optional, Type

import rich.progress


class Progress:
    def __init__(self):
        self._progress = rich.progress.Progress()
        self._task: rich.progress.TaskID | None = None

    def __call__(self, total_size: int, bytes_read: int):
        if self._task is None:
            self._task = self._progress.add_task("Downloading", total=total_size)
        self._progress.update(self._task, completed=bytes_read)

    def __enter__(self):
        self._progress.__enter__()
        return self

    def __exit__(
        self,
        exc_type: Optional[Type[BaseException]],
        exc_val: Optional[BaseException],
        exc_tb: Optional[TracebackType],
    ):
        self._progress.__exit__(exc_type, exc_val, exc_tb)
