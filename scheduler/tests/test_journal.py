import pytest

from deciphon_scheduler.journal import Journal
from deciphon_scheduler.settings import Settings


@pytest.mark.asyncio
async def test_journal(mosquitto):
    settings = Settings()
    journal = Journal(settings)

    async with journal:
        await journal.publish_hmm(1)
