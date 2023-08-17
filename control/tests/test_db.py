from pathlib import Path

from deciphonctl.cli import app


def test_add(runner, files_path: Path):
    minifam_hmm = files_path / "minifam.hmm"
    runner.invoke(app, ["hmm", "add", str(minifam_hmm), "1"])
    minifam_db = files_path / "minifam.dcp"
    assert runner.invoke(app, ["db", "add", str(minifam_db), "1"]).exit_code == 0


def test_ls(runner):
    assert runner.invoke(app, ["db", "ls"]).exit_code == 0


def test_rm(runner, files_path: Path):
    minifam_hmm = files_path / "minifam.hmm"
    runner.invoke(app, ["hmm", "add", str(minifam_hmm), "1"])
    minifam_db = files_path / "minifam.dcp"
    runner.invoke(app, ["db", "add", str(minifam_db), "1"])
    assert runner.invoke(app, ["db", "rm", "1"]).exit_code == 0
