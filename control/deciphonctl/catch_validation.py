from __future__ import annotations

from click.exceptions import UsageError
from pydantic import ValidationError


class catch_validation:
    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        del exc_tb
        if exc_type is ValidationError:
            input = exc_val.errors()[0]["input"]
            msg = exc_val.errors()[0]["msg"]
            raise UsageError(f"{msg} ({input})")
        return exc_type is None
