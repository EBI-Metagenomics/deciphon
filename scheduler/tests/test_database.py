from deciphon_scheduler.database import Database
from deciphon_scheduler.scheduler.models import metadata
from deciphon_scheduler.settings import Settings

TABLE_NAMES = ["job", "hmm", "db", "scan", "seq", "snap"]


def test_database_tables(settings: Settings):
    database = Database(settings)
    database.create_tables(metadata())
    table_names = [x for x in database.metadata().tables]
    assert set(table_names) == set(TABLE_NAMES)
