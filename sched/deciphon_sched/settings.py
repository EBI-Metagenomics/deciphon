from pydantic import AnyUrl
from pydantic_settings import BaseSettings, SettingsConfigDict


class Settings(BaseSettings):
    model_config = SettingsConfigDict(env_prefix="deciphon_sched_")

    database_url: AnyUrl = "sqlite+pysqlite:///:memory:"
    allow_origins: list[str] = ["http://127.0.0.1:8000"]
