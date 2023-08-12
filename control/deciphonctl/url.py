import urllib.parse
from deciphonctl import settings
from pydantic import HttpUrl


def url(endpoint: str):
    return urllib.parse.urljoin(settings.sched_url.unicode_string(), endpoint)


def url_filename(url: HttpUrl):
    path = url.path
    assert isinstance(path, str)
    return path.split("/")[-1]
