from requests.exceptions import JSONDecodeError

import rich
from typer import Exit


def handle_http_error(response):
    if not response.ok:
        try:
            rich.print(response.json())
            raise Exit(1)
        except JSONDecodeError:
            pass
    response.raise_for_status()
