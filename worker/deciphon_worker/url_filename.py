from pydantic import HttpUrl


def url_filename(url: HttpUrl):
    path = url.path
    assert isinstance(path, str)
    return path.split("/")[-1]
