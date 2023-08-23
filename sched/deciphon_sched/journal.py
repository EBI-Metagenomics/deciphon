from aiomqtt import Client
from deciphon_sched.logger import Logger

from deciphon_sched.settings import Settings


class Journal:
    def __init__(self, settings: Settings, logger: Logger):
        self._mqtt = Client(hostname=settings.mqtt_host, port=settings.mqtt_port)
        self._topic = settings.mqtt_topic
        self._logger = logger

    async def __aenter__(self):
        await self._mqtt.__aenter__()
        return self

    async def __aexit__(self, *args, **kargs):
        await self._mqtt.__aexit__(*args, **kargs)

    async def publish(self, subject: str, payload: str):
        topic = f"/{self._topic}/{subject}"
        self._logger.handler.info(f"publishing <{payload}> to <{topic}>")
        await self._mqtt.publish(topic, payload)
