import pytest

from deciphon_scheduler.journal import Journal
from deciphon_scheduler.settings import Settings


@pytest.mark.asyncio
async def test_journal(mqtt, settings: Settings):
    settings.mqtt_host = mqtt["host"]
    settings.mqtt_port = mqtt["port"]
    journal = Journal(settings)

    async with journal:
        await journal.publish("hmms", "1")
