from fastapi import HTTPException
from sqlalchemy.exc import IntegrityError
from starlette.requests import Request
from starlette.responses import JSONResponse
from starlette.status import (
    HTTP_404_NOT_FOUND,
    HTTP_422_UNPROCESSABLE_ENTITY,
)

__all__ = [
    "FileNotInStorageError",
    "InvalidSnapFileError",
    "NotFoundInSchedError",
    "integrity_error_handler",
]


async def integrity_error_handler(_: Request, exc: IntegrityError):
    return JSONResponse(
        content={"detail": str(exc)}, status_code=HTTP_422_UNPROCESSABLE_ENTITY
    )


class NotFoundInSchedError(HTTPException):
    def __init__(self, name: str):
        super().__init__(HTTP_404_NOT_FOUND, f"{name} not found in the scheduler")


class FileNotInStorageError(HTTPException):
    def __init__(self, sha256: str):
        super().__init__(
            HTTP_422_UNPROCESSABLE_ENTITY,
            f"File (SHA256: {sha256}) not found in storage",
        )


class InvalidSnapFileError(HTTPException):
    def __init__(self, sha256: str, reason: str):
        super().__init__(
            HTTP_422_UNPROCESSABLE_ENTITY,
            f"Invalid Snap file (SHA256: {sha256}). Reason: {reason}.",
        )
