import os
import shutil

from h3daemon.hmmfile import HMMFile, Path

from deciphon_worker import launch_hmmer, shutting


def test_hmmer_worker(tmp_path, files_path: Path):
    os.chdir(tmp_path)
    shutil.copy(files_path / "minifam.hmm", Path("minifam.hmm"))
    hmmfile = HMMFile(Path("minifam.hmm"))
    hmmer = launch_hmmer(hmmfile).result()
    with shutting(hmmer):
        hmmer.port
