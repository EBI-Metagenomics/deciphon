import pytest

from deciphon_scheduler.journal import Journal
from deciphon_scheduler.settings import Settings


@pytest.mark.asyncio
async def test_journal(mqtt, settings: Settings):
    settings.mqtt_host = mqtt["host"]
    settings.mqtt_port = mqtt["port"]
    async with Journal(settings) as journal:
        await journal.publish("hmms", "1")
