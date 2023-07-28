from sqlalchemy import MetaData, create_engine
from sqlalchemy.orm import Session

from .models import BaseModel
from .settings import Settings


class Database:
    def __init__(self, settings: Settings):
        self._engine = create_engine(settings.database_url.unicode_string(), echo=True)

    def create_tables(self):
        BaseModel.metadata.create_all(self._engine)

    def create_session(self):
        return Session(self._engine)

    def metadata(self):
        x = MetaData()
        x.reflect(self._engine)
        return x

    def dispose(self):
        self._engine.dispose()
