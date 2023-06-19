from __future__ import annotations

from functools import lru_cache

from fastapi_mqtt import FastMQTT, MQTTConfig

from deciphon_api.config import get_config

__all__ = ["get_journal", "Journal"]


@lru_cache
def get_journal() -> Journal:
    cfg = get_config()
    x = MQTTConfig(host=cfg.mqtt_host, port=cfg.mqtt_port)
    return Journal(FastMQTT(config=x))


class Journal:
    def __init__(self, mqtt):
        self._mqtt = mqtt

    @property
    def mqtt(self):
        return self._mqtt

    async def publish_hmm(self, hmm_id: int):
        self.mqtt.publish("/deciphon/hmm", str(hmm_id))

    async def publish_scan(self, scan_id: int):
        self.mqtt.publish("/deciphon/scan", str(scan_id))
