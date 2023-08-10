from pydantic import HttpUrl, AnyUrl
from pydantic_settings import BaseSettings, SettingsConfigDict


class Settings(BaseSettings):
    model_config = SettingsConfigDict(env_prefix="deciphon_sched_")

    endpoint_prefix: str = ""
    allow_origins: list[str] = ["http://127.0.0.1:8000"]

    database_url: AnyUrl = AnyUrl("sqlite+pysqlite:///:memory:")

    s3_key: str = "minioadmin"
    s3_secret: str = "minioadmin"
    s3_url: HttpUrl = HttpUrl("http://127.0.0.1:9000")
    s3_bucket: str = "deciphon"

    mqtt_host: str = "127.0.0.1"
    mqtt_port: int = 1883
    mqtt_topic: str = "deciphon"
