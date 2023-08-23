from deciphon_sched.database import Database
from deciphon_sched.logger import Logger
from deciphon_sched.sched.models import metadata
from deciphon_sched.settings import Settings

TABLE_NAMES = ["job", "hmm", "db", "scan", "seq", "snap"]


def test_database_tables(settings: Settings, logger: Logger):
    database = Database(settings, logger)
    database.create_tables(metadata())
    table_names = [x for x in database.metadata().tables]
    assert set(table_names) == set(TABLE_NAMES)
