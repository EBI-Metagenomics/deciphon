from pydantic import HttpUrl, TypeAdapter


def http_url(url: str) -> HttpUrl:
    return TypeAdapter(HttpUrl).validate_strings(url)
