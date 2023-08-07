from pydantic import HttpUrl
from pydantic_settings import BaseSettings, SettingsConfigDict


class Settings(BaseSettings):
    model_config = SettingsConfigDict(env_prefix="deciphonctl_")

    scheduler_url: HttpUrl = HttpUrl("http://127.0.0.1:8000")
