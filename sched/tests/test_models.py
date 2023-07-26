from sqlalchemy import select

from deciphon_sched.database import Database
from deciphon_sched.models import DB, HMM, Scan, HMMFileName
from deciphon_sched.settings import Settings

DATABASE_URL = "sqlite+pysqlite:///:memory:"


def test_models_add_get_hmm():
    settings = Settings(database_url=DATABASE_URL)
    database = Database(settings)
    database.create_tables()
    hmmfile = HMMFileName(name="file.hmm")

    with database.create_session() as session:
        for i in range(1, 2):
            hmm = HMM.create(file_name=hmmfile)

            session.add(hmm)
            session.commit()
            assert hmm.id == i
            assert hmm.job.id == i

            hmm = session.scalars(select(HMM).where(HMM.id == hmm.id)).one()
            assert hmm.id == i
            assert hmm.job.id == i


def test_models_add_get_db():
    settings = Settings(database_url=DATABASE_URL)
    database = Database(settings)
    database.create_tables()
    hmmfile = HMMFileName(name="file.hmm")

    with database.create_session() as session:
        for i in range(1, 2):
            db = DB.create(hmm=HMM.create(file_name=hmmfile))

            session.add(db)
            session.commit()
            assert db.id == i
            assert db.hmm.id == i
            assert db.hmm.job.id == i

            db = session.scalars(select(DB).where(DB.id == db.id)).one()
            assert db.id == i
            assert db.hmm.id == i
            assert db.hmm.job.id == i


def test_models_add_get_scan():
    settings = Settings(database_url=DATABASE_URL)
    database = Database(settings)
    database.create_tables()
    hmmfile = HMMFileName(name="file.hmm")

    with database.create_session() as session:
        for i in range(1, 2):
            scan = Scan.create(db=DB.create(hmm=HMM.create(file_name=hmmfile)))

            session.add(scan)
            session.commit()
            assert scan.id == i
            assert scan.job.id == i
            assert scan.db.id == i
            assert scan.db.hmm.id == i
            assert scan.db.hmm.job.id == i * 2
