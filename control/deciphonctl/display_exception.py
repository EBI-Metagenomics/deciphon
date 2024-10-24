from functools import wraps

from rich.console import Console
from rich.panel import Panel
from typer import Exit


def display_error(obj):
    err_console = Console(stderr=True)
    panel = Panel(obj, border_style="red", title="Error", title_align="left")
    err_console.print(panel)


def display_exception(exceptions, exit_code=1):
    def decorator(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            try:
                return func(*args, **kwargs)
            except exceptions as exc:
                display_error(str(exc))
                raise Exit(code=exit_code)

        return wrapper

    return decorator
