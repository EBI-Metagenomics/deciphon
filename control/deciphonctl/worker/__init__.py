import typer

from .main import presser

app = typer.Typer()


@app.command()
def press(num_workers: int = 1):
    presser(num_workers)
