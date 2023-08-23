from pydantic import HttpUrl, AnyUrl
from pydantic_settings import BaseSettings, SettingsConfigDict
from enum import Enum


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
    model_config = SettingsConfigDict(env_prefix="deciphon_sched_")

    endpoint_prefix: str = ""
    allow_origins: list[str] = ["http://localhost"]
    log_level: LogLevel = LogLevel.info

    database_url: AnyUrl = AnyUrl("sqlite+pysqlite:///:memory:")

    s3_key: str = "minioadmin"
    s3_secret: str = "minioadmin"
    s3_url: HttpUrl = HttpUrl("http://localhost:9000")
    s3_bucket: str = "deciphon"

    mqtt_host: str = "localhost"
    mqtt_port: int = 1883
    mqtt_topic: str = "deciphon"
