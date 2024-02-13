import os
from pathlib import Path

from deciphon_core.schema import NewSnapFile


def test_collision(tmp_path: Path):
    os.chdir(tmp_path)

    x0 = NewSnapFile.create_from_prefix("snap")
    x0.basename.mkdir()

    x1 = NewSnapFile.create_from_prefix("snap")
    x1.basename.mkdir()

    x2 = NewSnapFile.create_from_prefix("snap")
    x2.basename.mkdir()

    x0.make_archive()
    x1.make_archive()
    x2.make_archive()

    assert x0.basename.name == "snap"
    assert x1.basename.name == "snap.1"
    assert x2.basename.name == "snap.2"
