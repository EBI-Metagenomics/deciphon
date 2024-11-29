import os
import shutil

from deciphon_core.schema import Gencode, HMMFile
from h3daemon.hmmfile import Path

from deciphon_worker import press


def test_press_worker(tmp_path, files_path: Path):
    os.chdir(tmp_path)
    shutil.copy(files_path / "minifam.hmm", Path("minifam.hmm"))
    hmmfile = HMMFile(path=Path("minifam.hmm"))
    task = press(hmmfile, Gencode.BAPP, 0.01)
    dbfile = task.result()
    assert task.done
    assert task.progress == 100
    assert dbfile.path.name == "minifam.dcp"
