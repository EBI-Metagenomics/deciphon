from sqlalchemy import select

from deciphon_sched.database import Database
from deciphon_sched.models import DB, HMM, Scan, HMMFile, DBFile, Seq
from deciphon_sched.settings import Settings

DATABASE_URL = "sqlite+pysqlite:///:memory:"


def test_models_add_get_hmm():
    settings = Settings(database_url=DATABASE_URL)
    database = Database(settings)
    database.create_tables()
    sha256 = "fe305d9c09e123f987f49b9056e34c374e085d8831f815cc73d8ea4cdec84960"
    hmmfile = HMMFile(name="file.hmm", sha256=sha256)

    with database.create_session() as session:
        for i in range(1, 2):
            hmm = HMM.create(file=hmmfile)

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
    sha256 = "fe305d9c09e123f987f49b9056e34c374e085d8831f815cc73d8ea4cdec84960"
    hmmfile = HMMFile(name="file.hmm", sha256=sha256)
    sha256 = "b84800e4803006fccbea1634696383e228c4a20f6902ccfddaff7bf511f8e340"
    dbfile = DBFile(name="file.dcp", sha256=sha256)

    with database.create_session() as session:
        for i in range(1, 2):
            db = DB.create(hmm=HMM.create(file=hmmfile), file=dbfile)

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
    sha256 = "ae305d9c09e123f987f49b9056e34c374e085d8831f815cc73d8ea4cdec84960"
    hmmfile = HMMFile(name="file.hmm", sha256=sha256)
    sha256 = "b84800e4803006fccbea1634696383e228c4a20f6902ccfddaff7bf511f8e340"
    dbfile = DBFile(name="file.dcp", sha256=sha256)

    with database.create_session() as session:
        for i in range(1, 2):
            db = DB.create(hmm=HMM.create(file=hmmfile), file=dbfile)
            seqs = [Seq(name="seq1", data="ACGT"), Seq(name="seq2", data="CGA")]
            scan = Scan.create(db=db, seqs=seqs)

            session.add(scan)
            session.commit()
            assert scan.id == i
            assert scan.job.id == i
            assert scan.db.id == i
            assert scan.db.hmm.id == i
            assert scan.db.hmm.job.id == i * 2
            assert scan.seqs[0].scan.id == i
            assert scan.seqs[1].scan.id == i
            assert scan.seqs[0].name == seqs[0].name
            assert scan.seqs[1].name == seqs[1].name
