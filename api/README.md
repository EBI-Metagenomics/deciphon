# deciphon-api

## Dependencies

If you happen to be using a supported Linux environment (which is likely the case),
you would need:

- [Python](https://www.python.org) >=3.8
- [Pipx](https://pypa.github.io/pipx/) for easy installation and environment isolation. Feel free to use [Pip](https://pip.pypa.io/en/stable/) instead though.

## Usage

Generate a configuration file:

```bash
pipx run deciphon-api generate-config > .env
```

Tweak `.env` as needed, and then run

```bash
pipx run deciphon-api start
```

## Development

Make sure you have [Poetry](https://python-poetry.org/docs/).

Enter

```bash
poetry install
poetry shell
```

to setup and activate a Python environment associated with the project.
Then enter

```bash
uvicorn deciphon_api.main:app.api --reload
```

to start the API.

Tests can be performed by entering

```bash
pytest
```

while the corresponding Python environment created by Poetry is active.

## Settings

Copy the file [.env.example](.env.example) to your working directory and rename it to `.env`.
Edit it accordingly.
The rest of the configuration can be tuned by `uvicorn` options.
