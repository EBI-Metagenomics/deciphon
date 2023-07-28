from deciphon_scheduler.database import Database
from deciphon_scheduler.settings import Settings

TABLE_NAMES = ["job", "hmm", "db", "scan", "seq", "snap"]
DATABASE_URL = "sqlite+pysqlite:///:memory:"


def test_database_tables():
    settings = Settings(database_url=DATABASE_URL)
    database = Database(settings)
    database.create_tables()
    table_names = [x for x in database.metadata().tables]
    assert set(table_names) == set(TABLE_NAMES)
