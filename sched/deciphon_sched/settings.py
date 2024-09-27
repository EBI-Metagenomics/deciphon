from enum import Enum

from pydantic import AnyUrl, Field, HttpUrl
from pydantic_settings import BaseSettings, SettingsConfigDict


class LogLevel(str, Enum):
    debug = "debug"
    info = "info"
    warning = "warning"
    error = "error"
    critical = "critical"

    # Enum of Python3.10 returns a different string representation.
    # Make it return the same as in Python3.11
    def __str__(self):
        return str(self.value)


class Settings(BaseSettings):
    model_config = SettingsConfigDict(env_prefix="deciphon_sched_", env_file=".env")

    host: str = "0.0.0.0"
    port: int = 8000

    endpoint_prefix: str = ""
    allow_origins: list[str] = ["http://127.0.0.1", "http://localhost"]
    log_level: LogLevel = Field(default="info")

    database_url: AnyUrl = Field(default="sqlite+pysqlite:///:memory:")

    s3_key: str = "minioadmin"
    s3_secret: str = "minioadmin"
    s3_url: HttpUrl = Field(default="http://localhost:9000")
    s3_bucket: str = "deciphon"

    mqtt_host: str = "localhost"
    mqtt_port: int = 1883
    mqtt_topic: str = "deciphon"
