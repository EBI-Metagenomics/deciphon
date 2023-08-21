from pydantic import HttpUrl
from pydantic_settings import BaseSettings, SettingsConfigDict


class Settings(BaseSettings):
    model_config = SettingsConfigDict(env_prefix="deciphonctl_")

    sched_url: HttpUrl = HttpUrl("http://localhost")

    s3_key: str = "minioadmin"
    s3_secret: str = "minioadmin"
    s3_url: HttpUrl = HttpUrl("http://localhost:9000")
    s3_bucket: str = "deciphon"

    mqtt_host: str = "localhost"
    mqtt_port: int = 1883
    mqtt_topic: str = "deciphon"
