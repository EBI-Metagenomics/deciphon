import subprocess
import time

import boto3
import pytest
import requests


@pytest.fixture()
def s3server():
    host = "127.0.0.1"
    port = "9090"
    url = f"http://{host}:{port}"
    key = "s3_key"
    secret = "s3_secret"
    bucket = "test"
    try:
        if requests.get(url).ok:
            raise RuntimeError("moto server already up")
    except requests.exceptions.ConnectionError:
        pass

    proc = subprocess.Popen(
        ["moto_server", "s3", "-H", host, "-p", port],
        stdin=subprocess.DEVNULL,
    )

    timeout = 5
    ready = False
    while timeout > 0:
        try:
            if requests.get(url).ok:
                ready = True
                break
        except requests.exceptions.ConnectionError:
            pass
        timeout -= 0.1
        time.sleep(0.1)
    if not ready:
        raise RuntimeError("cannot connect to moto server")

    region_name = "eu-west-1"
    boto3.client(
        "s3",
        region_name=region_name,
        endpoint_url=url,
        aws_access_key_id=key,
        aws_secret_access_key=secret,
    ).create_bucket(
        Bucket=bucket,
        CreateBucketConfiguration={"LocationConstraint": region_name},
    )

    yield {"url": url, "key": key, "secret": secret, "bucket": bucket}
    proc.terminate()
    proc.wait()
