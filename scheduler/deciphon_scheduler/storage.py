from __future__ import annotations

import boto3
from botocore.exceptions import ClientError
from pydantic import AnyHttpUrl, BaseModel

from deciphon_scheduler.settings import Settings


class Storage:
    def __init__(self, settings: Settings):
        self._s3 = boto3.client(
            "s3",
            endpoint_url=settings.s3_url.unicode_string(),
            aws_access_key_id=settings.s3_key,
            aws_secret_access_key=settings.s3_secret,
        )
        self._bucket = settings.s3_bucket
        if not self._bucket_exists():
            self._s3.create_bucket(Bucket=self._bucket)

    def _bucket_exists(self):
        try:
            self._s3.head_bucket(Bucket=self._bucket)
            return True
        except ClientError as e:
            if e.response["Error"]["Code"] == "404":
                return False
            raise e

    def presigned_upload(self, file_name: str) -> PresignedUpload:
        x = self._s3.generate_presigned_post(
            self._bucket, file_name, Fields=None, Conditions=None, ExpiresIn=3600
        )
        return PresignedUpload(url=AnyHttpUrl(x["url"]), fields=x["fields"])

    def delete(self, file_name: str):
        self._s3.delete_object(Bucket=self._bucket, Key=file_name)

    def has_file(self, file_name: str) -> bool:
        try:
            self._s3.head_object(Bucket=self._bucket, Key=file_name)
            return True
        except ClientError as e:
            if e.response["Error"]["Code"] == "404":
                return False
            raise e


class PresignedUpload(BaseModel):
    url: AnyHttpUrl
    fields: dict[str, str]
