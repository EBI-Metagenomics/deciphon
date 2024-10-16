from typing import Optional

from pydantic import HttpUrl
from pydantic_settings import BaseSettings, SettingsConfigDict

from deciphonctl.url import http_url


class Settings(BaseSettings):
    model_config = SettingsConfigDict(env_prefix="deciphonctl_")

    sched_url: HttpUrl = http_url("http://localhost")
    s3_url: Optional[HttpUrl] = None
