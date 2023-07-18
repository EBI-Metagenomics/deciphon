from __future__ import annotations

from functools import lru_cache
from typing import Type, TypeVar

from sqlmodel import Session, SQLModel, create_engine, select

from deciphon_api.config import get_config
from deciphon_api.errors import NotFoundInSchedError

__all__ = ["Sched", "select", "get_sched"]


@lru_cache
def get_sched() -> Sched:
    config = get_config()
    uri = f"sqlite:///{config.sched_filename}"
    connect_args = {"check_same_thread": False}
    engine = create_engine(uri, echo=config.sql_echo, connect_args=connect_args)
    return Sched(engine)


_TSelectParam = TypeVar("_TSelectParam")


class Sched:
    def __init__(self, engine):
        self._engine = engine
        self._session = Session(self._engine)

    @property
    def engine(self):
        return self._engine

    def __enter__(self):
        return self

    def __exit__(self, *_):
        self._session.close()

    def add(self, x: SQLModel):
        self._session.add(x)

    def get(self, x: Type[_TSelectParam], ident) -> _TSelectParam:
        i = self._session.get(x, ident)
        if not i:
            raise NotFoundInSchedError(i.__class__.__name__)
        return i

    def exec(self, stmt):
        return self._session.exec(stmt)

    def commit(self):
        self._session.commit()

    def delete(self, x: SQLModel):
        self._session.delete(x)

    def refresh(self, x: SQLModel):
        self._session.refresh(x)

    def close(self):
        self._session.close()
