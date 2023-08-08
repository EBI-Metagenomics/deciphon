# import asyncio
# import signal
# from pathlib import Path
#
# from deciphon_core.press import Press
# from gmqtt import Client as MQTTClient
#
# from deciphon.api.api import get_api
# from deciphon.config import get_config
# from deciphon.models import HMM
# from deciphon.storage import storage_get, storage_put
#
# STOP = asyncio.Event()
#
#
# def on_connect(client, *_):
#     client.subscribe("/deciphon/hmm")
#
#
# def on_message(client, topic, payload, qos, properties):
#     del client
#     del topic
#     del qos
#     del properties
#
#     hmm = HMM.parse_raw(get_api().read_hmm(int(payload)))
#     print(f"Received: {hmm}")
#
#     storage_get(hmm.sha256, Path(hmm.filename))
#
#     db = Path(Path(hmm.filename).stem + ".dcp")
#     print(f"Pressing: {hmm.filename}")
#     with Press(hmm.filename, db) as press:
#         for _ in press:
#             pass
#
#     print(f"Publishing: {db}")
#     storage_put(db)
#     get_api().create_db(db)
#     print(f"Finished: {db}")
#
#
# def on_disconnect(*_):
#     print("disconnected")
#
#
# def on_subscribe(*_):
#     print("subscribed")
#
#
# def ask_exit(*_):
#     STOP.set()
#
#
# async def pressd():
#     loop = asyncio.get_running_loop()
#     loop.add_signal_handler(signal.SIGINT, ask_exit)
#     loop.add_signal_handler(signal.SIGTERM, ask_exit)
#
#     client = MQTTClient(None)
#
#     client.on_connect = on_connect
#     client.on_message = on_message
#     client.on_disconnect = on_disconnect
#     client.on_subscribe = on_subscribe
#
#     cfg = get_config()
#     await client.connect(cfg.mqtt_host, cfg.mqtt_port)
#     try:
#         await STOP.wait()
#     finally:
#         await client.disconnect()
