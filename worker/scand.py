import asyncio

# import json
# import os
# import shutil
# import signal
# from pathlib import Path
#
# from deciphon_core.scan import Scan as ScanCore
# from gmqtt import Client as MQTTClient
# from h3daemon.hmmfile import HMMFile
#
# from deciphon.api.api import get_api
# from deciphon.config import get_config
# from deciphon.models import DB, Scan
# from deciphon.storage import storage_get, storage_put

STOP = asyncio.Event()


# def on_connect(client, *_):
#     client.subscribe("/deciphon/scan")
#
#
# def on_message(client, topic, payload, qos, properties):
#     del client
#     del topic
#     del qos
#     del properties
#
#     scan = Scan.parse_raw(get_api().read_scan(int(payload)))
#     print(f"Received: {scan}")
#
#     db = DB.parse_raw(get_api().read_db(scan.db_id))
#     print(f"Using: {db}")
#
#     scan_seqs = scan.dict()["seqs"]
#     for i in scan_seqs:
#         i["scan_id"] = scan.id
#
#     seqs_tmp = Path("snap.json")
#     with open(seqs_tmp, "w", encoding="utf-8") as fp:
#         json.dump(scan_seqs, fp, ensure_ascii=True)
#
#     force = True
#
#     print(f"Fetching: {db.filename}")
#     storage_get(db.sha256, Path(db.filename))
#     dbfile = Path(db.filename)
#
#     hmmfile = Path(dbfile.stem + ".hmm")
#     hmmfiled = HMMFile(hmmfile)
#     with H3Manager() as h3:
#         print(f"Pressing: {hmmfile}")
#         hmmpress(hmmfiled)
#         pod = h3.start_daemon(hmmfiled, force=True)
#         with ScanCore(hmmfile, seqs_tmp, pod.host_port) as x:
#             if force:
#                 if Path(x.product_name).exists():
#                     os.unlink(x.product_name)
#
#                 if Path(x.base_name).exists():
#                     shutil.rmtree(x.base_name)
#             print(f"Scanning: {seqs_tmp}")
#             x.run()
#         pod.stop()
#
#     snap = seqs_tmp.stem + ".dcs"
#     print(f"Publishing: {snap}")
#     storage_put(Path(snap))
#     get_api().create_snap(scan.id, Path(snap))
#     print("Finished: " + seqs_tmp.stem + ".dcs")
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
# async def scand():
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
