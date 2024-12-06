import os
import shutil
from pathlib import Path

from deciphon_schema import Gencode, HMMFile

from deciphon_worker import press


def test_press(tmp_path, files_path: Path):
    os.chdir(tmp_path)
    shutil.copy(files_path / "minifam.hmm", Path("minifam.hmm"))
    hmmfile = HMMFile(path=Path("minifam.hmm"))
    task = press(hmmfile, Gencode.BAPP, 0.01)
    dbfile = task.result()
    assert task.done
    assert task.progress == 100
    assert dbfile.path.name == "minifam.dcp"
    assert os.path.getsize(dbfile.path) == 3609858
