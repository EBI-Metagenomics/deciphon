from fastapi import Depends
from fastapi.security import APIKeyHeader

from deciphon_api.config import get_config

__all__ = ["auth_request"]


def auth_request(token: str = Depends(APIKeyHeader(name="X-API-Key"))) -> bool:
    authenticated = token == get_config().key
    return authenticated
