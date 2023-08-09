from __future__ import annotations

import asyncio
import time

import typer
from aiomqtt.error import MqttError

from deciphon_worker import settings
from deciphon_worker.presser import Presser
from deciphon_worker.producer import Producer

app = typer.Typer()


async def forever_loop(producer: Producer):
    while True:
        try:
            await producer.run()
        except MqttError as exception:
            print(exception)
            print("Trying again in 2 seconds")
            time.sleep(2)


@app.command()
def press(num_workers: int):
    asyncio.run(forever_loop(Presser(settings, num_workers)))
