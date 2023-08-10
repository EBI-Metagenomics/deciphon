from deciphon_scheduler.scheduler.schemas import Gencode
import pytest
from sqlalchemy import select

from deciphon_scheduler.database import Database
from deciphon_scheduler.scheduler.models import (
    DB,
    HMM,
    DBFile,
    HMMFile,
    Scan,
    Seq,
    metadata,
)
from deciphon_scheduler.settings import Settings


@pytest.fixture()
def hmmfile():
    return HMMFile(name="file.hmm")


@pytest.fixture()
def dbfile():
    return DBFile(name="file.dcp", gencode=Gencode(1), epsilon=0.01)


@pytest.fixture()
def session(settings: Settings):
    database = Database(settings)
    database.create_tables(metadata())
    yield database.create_session()


def test_models_add_get_hmm(session, hmmfile: HMMFile):
    for i in range(1, 2):
        hmm = HMM.create(file=hmmfile)

        session.add(hmm)
        session.commit()
        assert hmm.id == i
        assert hmm.job.id == i

        hmm = session.scalars(select(HMM).where(HMM.id == hmm.id)).one()
        assert hmm.id == i
        assert hmm.job.id == i


def test_models_add_get_db(session, hmmfile: HMMFile, dbfile: DBFile):
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


def test_models_add_get_scan(session, hmmfile: HMMFile, dbfile: DBFile):
    for i in range(1, 2):
        db = DB.create(hmm=HMM.create(file=hmmfile), file=dbfile)
        seqs = [Seq(name="seq1", data="ACGT"), Seq(name="seq2", data="CGA")]
        scan = Scan.create(db, True, False, seqs)

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
        assert seqs[0].id == 1
        assert seqs[1].id == 2
        assert seqs[0].scan_id == scan.id
        assert seqs[1].scan_id == scan.id
