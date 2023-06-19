import fsspec
from fsspec.implementations.zip import ZipFileSystem

from deciphon_api.config import get_config

__all__ = ["snap_fs"]


def snap_fs(sha256: str):
    endpoint_url = f"https://{get_config().s3_host}"
    cls = fsspec.get_filesystem_class("s3")
    s3 = cls(anon=True, client_kwargs={"endpoint_url": endpoint_url})
    bucket = get_config().s3_bucket
    return ZipFileSystem(s3.open(f"/{bucket}/{sha256}"))
