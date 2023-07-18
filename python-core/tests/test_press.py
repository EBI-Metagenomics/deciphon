import os
import shutil
from pathlib import Path

from deciphon_core.hmmfile import HMMFile
from deciphon_core.press import Press


def test_press(tmp_path: Path, files_path: Path):
    shutil.copy(files_path / "minifam.hmm", tmp_path)
    os.chdir(tmp_path)

    hmmfile = HMMFile(path=Path("minifam.hmm"))
    with Press(hmmfile) as press:
        for x in press:
            x.press()

    assert hmmfile.dbfile.path.stat().st_size == 9933912
