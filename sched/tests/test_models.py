from sqlalchemy import select

from deciphon_sched.database import Database
from deciphon_sched.models import DB, HMM, Job, Scan
from deciphon_sched.settings import Settings

DATABASE_URL = "sqlite+pysqlite:///:memory:"


def test_models_add_get_job():
    settings = Settings(database_url=DATABASE_URL)
    database = Database(settings)
    database.create_tables()

    with database.create_session() as session:
        for i in range(1, 2):
            job = Job()

            assert job.id is None
            session.add(job)
            session.commit()
            assert job.id == i

            job = session.scalars(select(Job).where(Job.id == job.id)).one()
            assert job.id == i


def test_models_add_get_hmm():
    settings = Settings(database_url=DATABASE_URL)
    database = Database(settings)
    database.create_tables()

    with database.create_session() as session:
        for i in range(1, 2):
            hmm = HMM(job=Job())

            assert hmm.id is None
            assert hmm.job.id is None
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

    with database.create_session() as session:
        for i in range(1, 2):
            db = DB(hmm=HMM(job=Job()))

            assert db.id is None
            assert db.hmm.id is None
            assert db.hmm.job.id is None
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

    with database.create_session() as session:
        for i in range(1, 2):
            scan = Scan(job=Job(), db=DB(hmm=HMM(job=Job())))

            assert scan.id is None
            assert scan.job.id is None
            assert scan.db.id is None
            assert scan.db.hmm.id is None
            assert scan.db.hmm.job.id is None
            session.add(scan)
            session.commit()
            assert scan.id == i
            assert scan.job.id == i
            assert scan.db.id == i
            assert scan.db.hmm.id == i
            assert scan.db.hmm.job.id == i * 2
