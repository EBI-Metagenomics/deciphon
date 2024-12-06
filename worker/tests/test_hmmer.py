import os
import shutil
from pathlib import Path

from deciphon_schema import HMMFile

from deciphon_worker import launch_hmmer


def test_hmmer(tmp_path, files_path: Path):
    os.chdir(tmp_path)
    shutil.copy(files_path / "minifam.hmm", Path("minifam.hmm"))
    hmmfile = HMMFile(path=Path("minifam.hmm"))
    future = launch_hmmer(hmmfile)
    hmmer = future.result()
    with hmmer:
        hmmer.port
