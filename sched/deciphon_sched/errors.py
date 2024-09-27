from fastapi import HTTPException
from sqlalchemy.exc import IntegrityError
from starlette.requests import Request
from starlette.responses import JSONResponse
from starlette.status import (
    HTTP_400_BAD_REQUEST,
    HTTP_404_NOT_FOUND,
    HTTP_422_UNPROCESSABLE_ENTITY,
)


async def integrity_error_handler(_: Request, exc: Exception):
    assert isinstance(exc, IntegrityError)
    return JSONResponse(
        content={"detail": str(exc)}, status_code=HTTP_422_UNPROCESSABLE_ENTITY
    )


class FileNameExistsError(HTTPException):
    def __init__(self, name: str):
        super().__init__(
            HTTP_422_UNPROCESSABLE_ENTITY, f"File name '{name}' already exists"
        )


class FileNameNotFoundError(HTTPException):
    def __init__(self, name: str):
        super().__init__(HTTP_422_UNPROCESSABLE_ENTITY, f"File name '{name}' not found")


class NotFoundInDatabaseError(HTTPException):
    def __init__(self, name: str):
        super().__init__(HTTP_404_NOT_FOUND, f"'{name}' not found in the database")


class FoundInDatabaseError(HTTPException):
    def __init__(self, name: str):
        super().__init__(
            HTTP_422_UNPROCESSABLE_ENTITY, f"'{name}' already exists in the database"
        )


class JobStateTransitionError(HTTPException):
    def __init__(self, previous: str, next: str):
        super().__init__(HTTP_400_BAD_REQUEST, f"{previous}->{next} is invalid")


class SnapFileError(HTTPException):
    def __init__(self, msg: str):
        super().__init__(HTTP_400_BAD_REQUEST, f"Snap file is invalid: {msg}")
