# import json
import os
import shutil
import tempfile
from pathlib import Path

import pytest
from fastapi import FastAPI


@pytest.fixture
def app() -> FastAPI:
    from deciphon_api.main import get_app

    return get_app()


@pytest.fixture
def cleandir():
    oldcwd = Path(os.getcwd())
    newpath = Path(tempfile.mkdtemp())
    os.chdir(newpath)
    oldenv = oldcwd / ".env"
    newenv = newpath / ".env"
    if oldenv.exists():
        shutil.copy(oldenv, newenv)
    yield
    os.chdir(oldcwd)
    shutil.rmtree(newpath)
