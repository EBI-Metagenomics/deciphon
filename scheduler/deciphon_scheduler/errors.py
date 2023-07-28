from sqlalchemy.exc import IntegrityError
from fastapi import HTTPException
from starlette.requests import Request
from starlette.responses import JSONResponse
from starlette.status import HTTP_422_UNPROCESSABLE_ENTITY


async def integrity_error_handler(_: Request, exc: IntegrityError):
    return JSONResponse(
        content={"detail": str(exc)}, status_code=HTTP_422_UNPROCESSABLE_ENTITY
    )


class FileNameExistsError(HTTPException):
    def __init__(self, name: str):
        super().__init__(
            HTTP_422_UNPROCESSABLE_ENTITY, f"File name '{name}' already exists"
        )
