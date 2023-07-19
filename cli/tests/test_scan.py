import os
import shutil
from pathlib import Path

from typer.testing import CliRunner

from deciphon.cli import app

runner = CliRunner()


def test_scan(tmp_path: Path, files_path: Path):
    shutil.copy(files_path / "minifam.hmm", tmp_path)
    shutil.copy(files_path / "sequences.fna", tmp_path)
    os.chdir(tmp_path)

    result = runner.invoke(app, ["press", "minifam.hmm", "1"])
    assert result.exit_code == 0

    result = runner.invoke(
        app,
        [
            "scan",
            "minifam.hmm",
            "sequences.fna",
            "--snapfile",
            "snap_file.dcs",
            "--lrt-threshold",
            "2",
            "--num-threads",
            "1",
        ],
    )
    assert result.exit_code == 0
    assert Path("snap_file.dcs").exists()
