from __future__ import annotations

import pytest
from testcontainers.core.container import DockerContainer
from testcontainers.minio import MinioContainer
import paho.mqtt.client

from deciphon_scheduler.settings import Settings
from testcontainers.core.waiting_utils import wait_container_is_ready


@pytest.fixture(scope="package")
def settings():
    return Settings()


@pytest.fixture(scope="package")
def mqtt():
    with MosquittoContainer() as x:
        yield {
            "host": x.get_container_host_ip(),
            "port": int(x.get_exposed_port(x.port)),
        }


@pytest.fixture(scope="package")
def s3():
    with MinioContainer() as x:
        yield {
            "access_key": x.get_config()["access_key"],
            "secret_key": x.get_config()["secret_key"],
            "url": "http://" + x.get_config()["endpoint"],
        }


class MosquittoContainer(DockerContainer):
    def __init__(self, image="eclipse-mosquitto:2.0.15", port=1883):
        super(MosquittoContainer, self).__init__(image)
        self.port = port
        self.with_exposed_ports(self.port)

    @wait_container_is_ready(ConnectionError)
    def _healthcheck(self):
        host = self.get_container_host_ip()
        port = int(self.get_exposed_port(self.port))
        paho.mqtt.client.Client().connect(host, port, 15)

    def start(self):
        self.with_command("mosquitto -c /mosquitto-no-auth.conf")
        super().start()
        self._healthcheck()
        return self
