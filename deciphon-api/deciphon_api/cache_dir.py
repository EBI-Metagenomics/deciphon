import os
from functools import lru_cache
from platformdirs import user_cache_dir
from pathlib import Path

__all__ = ["get_cache_dir"]


@lru_cache
def get_cache_dir() -> Path:
    dir = user_cache_dir(__package__)
    os.makedirs(dir, exist_ok=True)
    return Path(dir)
