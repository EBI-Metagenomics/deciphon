from aiomqtt import Client
from loguru import logger

from deciphon_sched.settings import Settings


class Journal:
    def __init__(self, settings: Settings):
        self._mqtt = Client(hostname=settings.mqtt_host, port=settings.mqtt_port)
        self._topic = settings.mqtt_topic

    async def __aenter__(self):
        await self._mqtt.__aenter__()
        return self

    async def __aexit__(self, *args, **kargs):
        await self._mqtt.__aexit__(*args, **kargs)

    async def publish(self, subject: str, payload: str):
        topic = f"/{self._topic}/{subject}"
        logger.info(f"publishing <{payload}> to <{topic}>")
        await self._mqtt.publish(topic, payload)
