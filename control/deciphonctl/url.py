import urllib.parse
from deciphonctl import settings


def url(endpoint: str):
    return urllib.parse.urljoin(settings.scheduler_url.unicode_string(), endpoint)
