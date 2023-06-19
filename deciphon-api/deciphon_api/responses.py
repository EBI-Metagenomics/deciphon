import json
import typing

from fastapi.responses import Response

from deciphon_api.mime import JSON

__all__ = ["PrettyJSONResponse"]


class PrettyJSONResponse(Response):
    media_type = JSON

    def render(self, content: typing.Any) -> bytes:
        return json.dumps(
            content,
            ensure_ascii=False,
            allow_nan=False,
            indent=2,
            separators=(", ", ": "),
        ).encode("utf-8")
