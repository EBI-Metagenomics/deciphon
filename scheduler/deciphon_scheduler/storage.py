import boto3

from .settings import Settings


class Storage:
    def __init__(self, settings: Settings):
        self._s3 = boto3.client(
            "s3",
            endpoint_url=settings.s3_url.unicode_string(),
            aws_access_key_id=settings.s3_key,
            aws_secret_access_key=settings.s3_secret,
        )
        self._bucket = settings.s3_bucket

    def presigned_upload(self, object_name: str):
        return self._s3.generate_presigned_post(
            self._bucket, object_name, Fields=None, Conditions=None, ExpiresIn=3600
        )
