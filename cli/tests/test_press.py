import os
import shutil
from pathlib import Path

from typer.testing import CliRunner

from deciphon.cli import app

runner = CliRunner()


def test_press(tmp_path: Path, files_path: Path):
    shutil.copy(files_path / "minifam.hmm", tmp_path)
    os.chdir(tmp_path)

    result = runner.invoke(app, ["press", "minifam.hmm", "11"])
    assert result.exit_code == 0

    assert Path("minifam.dcp").stat().st_size == 3536729
