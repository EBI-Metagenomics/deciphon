from pydantic import AnyUrl
from pydantic_settings import BaseSettings, SettingsConfigDict


class Settings(BaseSettings):
    model_config = SettingsConfigDict(env_prefix="deciphon_scheduler_")

    endpoint_prefix: str = ""
    allow_origins: list[str] = ["http://127.0.0.1:8000"]

    database_url: AnyUrl = "sqlite+pysqlite:///:memory:"

    s3_key: str = "s3_key"
    s3_secret: str = "s3_secret"
    s3_url: AnyUrl = "http://127.0.0.1:9090"
    s3_bucket: str = "deciphon"

    mqtt_broker: str = "127.0.0.1"
    mqtt_port: int = 1883
    mqtt_topic: str = "deciphon"
