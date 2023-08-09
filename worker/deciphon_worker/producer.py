import json
from abc import ABC, abstractmethod
from multiprocessing import JoinableQueue, Process
from typing import Any

from aiomqtt import Client
from pydantic import Json


class Producer(ABC):
    def __init__(self, mqtt_host: str, mqtt_port: int, topic: str, num_consumers: int):
        self._queue = JoinableQueue()
        self._topic = topic
        self._client = Client(mqtt_host, mqtt_port)
        self._consumers = [
            Process(target=self._consumer, args=(self._queue,), daemon=True)
            for _ in range(num_consumers)
        ]

    def _consumer(self):
        while True:
            message = self._queue.get()
            if not isinstance(message, str):
                continue
            try:
                json.loads(message)
            except json.JSONDecodeError:
                continue
            self.consume(message)
            self._queue.task_done()

    @abstractmethod
    def consume(self, message: Json[Any]):
        ...

    async def __aenter__(self):
        await self._client.__aenter__()

    async def __aexit__(self, *args):
        await self._client.__aexit__(*args)

    async def run(self):
        async with self._client.messages() as messages:
            await self._client.subscribe(self._topic)
            async for x in messages:
                assert isinstance(x.payload, bytes)
                self._queue.put(x.payload.decode())
