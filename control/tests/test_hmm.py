from pathlib import Path

from deciphonctl.cli import app


def test_add(runner, files_path: Path):
    minifam_hmm = files_path / "minifam.hmm"
    assert runner.invoke(app, ["hmm", "add", str(minifam_hmm), "1"]).exit_code == 0


def test_ls(runner):
    assert runner.invoke(app, ["hmm", "ls"]).exit_code == 0


def test_rm(runner, files_path: Path):
    minifam_hmm = files_path / "minifam.hmm"
    runner.invoke(app, ["hmm", "add", str(minifam_hmm), "1"])
    assert runner.invoke(app, ["hmm", "rm", "1"]).exit_code == 0
