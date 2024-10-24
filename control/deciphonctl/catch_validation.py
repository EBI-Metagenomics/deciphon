from functools import wraps

from click.exceptions import UsageError
from pydantic import ValidationError


def catch_validation(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        try:
            return func(*args, **kwargs)
        except ValidationError as e:
            input = e.errors()[0]["input"]
            msg = e.errors()[0]["msg"]
            raise UsageError(f"{msg} ({input})")

    return wrapper
