import os
import shutil
from pathlib import Path

from deciphon_schema import Gencode, HMMFile

from deciphon_core.press import PressContext


def test_press(tmp_path: Path, files_path: Path):
    shutil.copy(files_path / "minifam.hmm", tmp_path)
    os.chdir(tmp_path)

    hmmfile = HMMFile(path=Path("minifam.hmm"))
    with PressContext(hmmfile, Gencode(1)) as press:
        while not press.end():
            press.next()

    assert hmmfile.dbpath.path.stat().st_size == 3609858
