from functools import wraps

from rich.console import Console
from rich.panel import Panel
from rich.pretty import Pretty
from typer import Exit


def display_error(obj):
    err_console = Console(stderr=True)
    panel = Panel(Pretty(obj), border_style="red", title="Error", title_align="left")
    err_console.print(panel)


def display_exception(exceptions, exit_code=1):
    def decorator(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            try:
                return func(*args, **kwargs)
            except exceptions as e:
                display_error(e)
                raise Exit(code=exit_code)

        return wrapper

    return decorator
