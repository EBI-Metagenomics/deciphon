import importlib.metadata
import logging
from functools import lru_cache
from typing import Any, Dict, List, Tuple

from loguru import logger
from pydantic import BaseSettings

from deciphon_api.logging import InterceptHandler, LoggingLevel, RepeatMessageHandler

__all__ = ["Config", "get_config"]


FMT0 = "<green>{time:YYYY-MM-DD HH:mm:ss.SSS}</green> | "
FMT1 = "<level>{level: <8}</level> | "
FMT2L = "<cyan>{name}</cyan>:<cyan>{function}</cyan>:<cyan>{line}</cyan>"
FMT2R = " - <level>{message}</level>"
FMT2 = FMT2L + FMT2R


class Config(BaseSettings):
    debug: bool = False
    docs_url: str = "/docs"
    openapi_prefix: str = ""
    openapi_url: str = "/openapi.json"
    redoc_url: str = "/redoc"
    title: str = "Deciphon API"
    version: str = importlib.metadata.version(__package__)

    prefix: str = ""
    key: str = "change-me"

    host: str = "127.0.0.1"
    port: int = 49329

    allowed_hosts: List[str] = ["*"]

    mqtt_host = "127.0.0.1"
    mqtt_port = 1883

    s3_host = "s3.example.com"
    s3_bucket = "blx"

    h3result = "/path/to/h3client"

    sql_echo: bool = False
    logging_level: LoggingLevel = LoggingLevel("info")
    loggers: Tuple[str, str] = ("uvicorn.asgi", "uvicorn.access")
    # Refer to loguru format for details.
    logging_format: str = FMT0 + FMT1 + FMT2

    sched_filename: str = "deciphon.sched"
    reload: bool = False

    class Config:
        env_prefix = "dcp_api_"
        env_file = ".env"
        env_file_encoding = "utf-8"
        validate_assignment = True

    @property
    def fastapi_kwargs(self) -> Dict[str, Any]:
        return {
            "debug": self.debug,
            "docs_url": self.docs_url,
            "openapi_prefix": self.openapi_prefix,
            "openapi_url": self.openapi_url,
            "redoc_url": self.redoc_url,
            "title": self.title,
            "version": self.version,
        }

    def configure_logging(self) -> None:
        logging.getLogger().handlers = [InterceptHandler()]
        for logger_name in self.loggers:
            logging_logger = logging.getLogger(logger_name)
            logging_logger.handlers = [InterceptHandler(level=self.logging_level.level)]

        logger.configure(
            handlers=[
                {
                    "sink": RepeatMessageHandler(),
                    "level": self.logging_level.level,
                    "format": self.logging_format,
                }
            ]
        )


@lru_cache
def get_config() -> Config:
    return Config()
