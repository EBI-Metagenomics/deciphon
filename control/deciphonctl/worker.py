import time
from multiprocessing import JoinableQueue, Process

import paho.mqtt.subscribe as subscribe
from loguru import logger

from deciphonctl.settings import Settings


def on_message(client, queue: JoinableQueue, x):
    del client
    assert isinstance(x.payload, bytes)
    payload = x.payload.decode()
    logger.info(f"received <{payload}>")
    queue.put(payload)


def worker_loop(
    settings: Settings, topic: str, queue: JoinableQueue, consumers: list[Process]
):
    for x in consumers:
        x.start()

    while True:
        try:
            host = settings.mqtt_host
            port = settings.mqtt_port
            logger.info(f"connecting to MQTT {host}:{port}")
            subscribe.callback(on_message, [topic], 0, queue, host, port)
        except KeyboardInterrupt:
            logger.info("shutdown requested")
            break
        except Exception as exception:
            logger.exception(exception)
            time.sleep(1)

    for x in consumers:
        x.kill()
        x.join()

    logger.info("goodbye!")
