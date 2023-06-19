from __future__ import annotations

from functools import lru_cache
from typing import Callable, TypeAlias

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from loguru import logger
from sqlalchemy.exc import IntegrityError
from sqlmodel import SQLModel

from deciphon_api.config import get_config
from deciphon_api.errors import integrity_error_handler
from deciphon_api.journal import get_journal
from deciphon_api.sched import get_sched

__all__ = ["get_app", "App"]

App: TypeAlias = FastAPI


def create_start_handler() -> Callable:
    async def start_app() -> None:
        SQLModel.metadata.create_all(get_sched().engine)

    return start_app


def create_stop_handler() -> Callable:
    @logger.catch
    async def stop_app() -> None:
        get_sched.cache_clear()
        get_journal.cache_clear()
        get_config.cache_clear()
        get_app.cache_clear()

    return stop_app


@lru_cache
def get_app() -> App:
    from deciphon_api.api.api import router as api_router

    config = get_config()
    config.configure_logging()

    app = FastAPI(**config.fastapi_kwargs)

    app.add_middleware(
        CORSMiddleware,
        allow_origins=config.allowed_hosts,
        allow_credentials=True,
        allow_methods=["*"],
        allow_headers=["*"],
    )

    app.add_event_handler(
        "startup",
        create_start_handler(),
    )
    app.add_event_handler(
        "shutdown",
        create_stop_handler(),
    )

    app.add_exception_handler(IntegrityError, integrity_error_handler)

    app.include_router(api_router, prefix=config.prefix)

    get_journal().mqtt.init_app(app)

    return app
