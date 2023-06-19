from functools import lru_cache

from pydantic import BaseSettings

__all__ = ["Config", "get_config"]


class Config(BaseSettings):
    api_proto: str = "http"
    api_host: str = "127.0.0.1"
    api_port: int = 8000
    api_prefix: str = ""
    api_key: str = "change-me"

    mqtt_host = "127.0.0.1"
    mqtt_port = 1883

    class Config:
        env_file = ".env"
        env_file_encoding = "utf-8"
        validate_assignment = True


@lru_cache
def get_config() -> Config:
    return Config()
