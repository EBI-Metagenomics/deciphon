from __future__ import annotations

from sqlalchemy.orm import DeclarativeBase


class BaseModel(DeclarativeBase):
    def model_dump(self):
        return {field.name: getattr(self, field.name) for field in self.__table__.c}

    def __repr__(self):
        params = ", ".join(f"{k}={v}" for k, v in self.model_dump().items())
        return f"{self.__class__.__name__}({params})"

    def __str__(self):
        return repr(self)
