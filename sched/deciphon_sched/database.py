from sqlalchemy import MetaData, create_engine
from sqlalchemy.orm import Session
from deciphon_sched.logger import Logger

from deciphon_sched.settings import Settings


class Database:
    def __init__(self, settings: Settings, logger: Logger):
        self._engine = create_engine(settings.database_url.unicode_string())
        self._logger = logger

    def create_tables(self, metadata: MetaData):
        self._logger.handler.debug("creating database tables")
        metadata.create_all(self._engine)

    def create_session(self):
        return Session(self._engine)

    def metadata(self):
        x = MetaData()
        x.reflect(self._engine)
        return x

    def dispose(self):
        self._logger.handler.debug("disposing database engine")
        self._engine.dispose()
