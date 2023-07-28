from deciphon_sched.settings import Settings
from deciphon_sched.storage import Storage


def test_storage(s3server):
    settings = Settings(
        s3_url=s3server["url"],
        s3_key=s3server["key"],
        s3_secret=s3server["secret"],
        s3_bucket=s3server["bucket"],
    )
    storage = Storage(settings)

    with storage.open("example.txt", "w") as f:
        f.write("content")

    with storage.open("example.txt", "r") as f:
        assert f.read() == "content"
