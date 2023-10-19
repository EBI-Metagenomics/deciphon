from __future__ import annotations

import time
import sys
import paho.mqtt.client
from testcontainers.core.container import DockerContainer
from testcontainers.core.waiting_utils import wait_container_is_ready
from testcontainers.minio import MinioContainer


def mqtt_server():
    with MosquittoContainer() as x:
        yield {
            "host": x.get_container_host_ip(),
            "port": int(x.get_exposed_port(x.port)),
        }


def s3_server():
    with MinioContainer() as x:
        yield {
            "client": x.get_client(),
            "access_key": x.get_config()["access_key"],
            "secret_key": x.get_config()["secret_key"],
            "url": "http://" + x.get_config()["endpoint"],
        }


def s3_cleanup(client):
    for bucket in client.list_buckets():
        for x in client.list_objects(bucket.name):
            client.remove_object(bucket.name, x.object_name)
        client.remove_bucket(bucket.name)


class MosquittoContainer(DockerContainer):
    def __init__(self, image="eclipse-mosquitto:2", port=1883):
        super(MosquittoContainer, self).__init__(image)
        self.port = port
        self.with_exposed_ports(self.port)

    @wait_container_is_ready(ConnectionError)
    def _healthcheck(self):
        sys.stderr.write(f"_healthcheck:begin:{time.time()}\n")
        host = self.get_container_host_ip()
        port = int(self.get_exposed_port(self.port))
        paho.mqtt.client.Client().connect(host, port, 5)
        sys.stderr.write(f"_healthcheck:end:{time.time()}\n")

    def start(self):
        self.with_command("mosquitto -c /mosquitto-no-auth.conf")
        super().start()
        self._healthcheck()
        return self
