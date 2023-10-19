import time
import sys
import pytest

from deciphon_sched.journal import Journal
from deciphon_sched.logger import Logger
from deciphon_sched.settings import Settings


@pytest.mark.asyncio
async def test_journal(mqtt, settings: Settings, logger: Logger):
    sys.stderr.write(f"test_journal:{time.time()}\n")
    settings.mqtt_host = mqtt["host"]
    settings.mqtt_port = mqtt["port"]
    async with Journal(settings, logger) as journal:
        await journal.publish("press", "1")
