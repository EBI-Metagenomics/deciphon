from requests import JSONDecodeError
from requests.models import HTTPError


class PosterHTTPError(HTTPError):
    def __init__(self, response):
        try:
            response.raise_for_status()
            assert False
        except HTTPError as x:
            msg = x.args[0]
            try:
                info = response.json()
            except JSONDecodeError:
                info = response.text
            super().__init__(msg + f" returned: {info}", response=response)
