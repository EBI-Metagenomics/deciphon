from typing import Annotated

from pydantic import BaseModel, Field


class Epsilon(BaseModel):
    value: Annotated[float, Field(strict=True, gt=0, lt=1)]
