from pydantic import HttpUrl, TypeAdapter


def url_filename(url: HttpUrl):
    path = url.path
    assert isinstance(path, str)
    return path.split("/")[-1]


def http_url(url: str) -> HttpUrl:
    return TypeAdapter(HttpUrl).validate_strings(url)
