import os
from enum import Enum
from pathlib import Path
from typing import Optional

import dotenv
from pydantic import HttpUrl
from pydantic_settings import BaseSettings, SettingsConfigDict
from xdg_base_dirs import xdg_config_home

from deciphonctl.url import http_url

ENV_PREFIX = "DECIPHONCTL_"
CFG_DIR = "deciphonctl"
CFG_FILE = "deciphonctl.conf"
CFG_MODE = 0o640


def cfg_file_set(file: Path, option: str, value: str):
    if not cfg_file().exists():
        cfg_file().touch(mode=CFG_MODE)
    dotenv.set_key(file, option, value)


def cfg_file():
    return xdg_config_home() / CFG_DIR / CFG_FILE


def cfg_vars():
    return dotenv.dotenv_values(cfg_file())


def env_vars():
    vars: dict[str, str] = {}
    for key, val in os.environ.items():
        if key.upper().startswith(ENV_PREFIX):
            vars[key] = val
    return vars


class Settings(BaseSettings):
    model_config = SettingsConfigDict(env_prefix=ENV_PREFIX, env_file=cfg_file())

    sched_url: HttpUrl = http_url("http://localhost")
    s3_url: Optional[HttpUrl] = None


class SettingsFields(str, Enum):
    sched_url = "sched_url"
    s3_url = "s3_url"


assert list(sorted(Settings.__fields__.keys())) == list(
    sorted([x.value for x in SettingsFields])
)
