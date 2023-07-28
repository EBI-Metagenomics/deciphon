from s3fs import S3FileSystem

from .settings import Settings


class Storage:
    def __init__(self, settings: Settings):
        self._s3: S3FileSystem = S3FileSystem(
            key=settings.s3_key,
            secret=settings.s3_secret,
            endpoint_url=settings.s3_url.unicode_string(),
            asynchronous=False,
        )
        self._bucket = settings.s3_bucket

    def open(self, file_name: str, mode: str):
        return self._s3.open(f"{self._bucket}/{file_name}", mode)
