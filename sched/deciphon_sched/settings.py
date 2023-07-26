from typing import Optional
from pydantic import AnyUrl
from pydantic_settings import BaseSettings, SettingsConfigDict


class Settings(BaseSettings):
    model_config = SettingsConfigDict(env_prefix="deciphon_sched_")

    database_url: AnyUrl = "sqlite+pysqlite:///:memory:"
    allow_origins: list[str] = ["http://127.0.0.1:8000"]

    s3_key: Optional[str] = None
    s3_secret: Optional[str] = None
    s3_url: AnyUrl = "http://127.0.0.1:9090"
    s3_bucket: str = "deciphon"
