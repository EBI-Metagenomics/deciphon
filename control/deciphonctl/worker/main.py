import time
from multiprocessing import JoinableQueue, Process

import paho.mqtt.subscribe as subscribe
from loguru import logger

from deciphonctl import settings
from deciphonctl.worker.presser import Presser
from deciphonctl.worker.progress_informer import ProgressInformer


def on_message(client, queue: JoinableQueue, x):
    del client
    assert isinstance(x.payload, bytes)
    payload = x.payload.decode()
    logger.info(f"received <{payload}>")
    queue.put(payload)


def main(queue: JoinableQueue, consumers: list[Process]):
    for x in consumers:
        x.start()

    while True:
        try:
            topic = f"/{settings.mqtt_topic}/press"
            host = settings.mqtt_host
            port = settings.mqtt_port
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


def presser(num_workers: int):
    qin = JoinableQueue()
    qout = JoinableQueue()
    informer = ProgressInformer(settings, qout)
    pressers = [Presser(settings, qin, qout) for _ in range(num_workers)]
    consumers = [Process(target=x.entry_point, daemon=True) for x in pressers]
    consumers += [Process(target=informer.entry_point, daemon=True)]
    main(qin, consumers)
