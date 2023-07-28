from contextlib import asynccontextmanager
from typing import Optional

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from sqlalchemy.exc import IntegrityError

from .database import Database
from .errors import integrity_error_handler
from .journal import Journal
from .scheduler import router
from .settings import Settings
from .storage import Storage


@asynccontextmanager
async def lifespan(app: FastAPI):
    settings: Settings = app.state.settings
    app.state.database = Database(settings)
    app.state.database.create_tables()
    app.state.storage = Storage(settings)
    app.state.journal = Journal(settings)
    yield
    app.state.database.dispose()


def create_app(settings: Optional[Settings] = None):
    settings = settings or Settings()
    app = FastAPI(lifespan=lifespan)
    app.state.settings = settings

    app.include_router(router, prefix=settings.endpoint_prefix)

    app.add_exception_handler(IntegrityError, integrity_error_handler)

    app.add_middleware(
        CORSMiddleware,
        allow_origins=settings.allow_origins,
        allow_credentials=True,
        allow_methods=["*"],
        allow_headers=["*"],
    )

    return app
