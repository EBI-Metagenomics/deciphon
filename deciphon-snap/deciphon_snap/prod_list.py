from typing import List

from pydantic import BaseModel

from deciphon_snap.prod import Prod

__all__ = ["ProdList"]


class ProdList(BaseModel):
    __root__: List[Prod]

    def __len__(self):
        return len(self.__root__)

    def __getitem__(self, i) -> Prod:
        return self.__root__[i]

    def __iter__(self):
        return iter(self.__root__)
