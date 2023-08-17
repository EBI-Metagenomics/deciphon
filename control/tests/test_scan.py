from pathlib import Path

from deciphonctl.cli import app


def test_add(runner, files_path: Path):
    minifam_hmm = files_path / "minifam.hmm"
    runner.invoke(app, ["hmm", "add", str(minifam_hmm), "1"])
    minifam_db = files_path / "minifam.dcp"
    runner.invoke(app, ["db", "add", str(minifam_db), "1"])
    consensus = files_path / "consensus.fna"
    assert runner.invoke(app, ["scan", "add", str(consensus), "1"]).exit_code == 0


def test_ls(runner):
    assert runner.invoke(app, ["scan", "ls"]).exit_code == 0


def test_rm(runner, files_path: Path):
    minifam_hmm = files_path / "minifam.hmm"
    runner.invoke(app, ["hmm", "add", str(minifam_hmm), "1"])
    minifam_db = files_path / "minifam.dcp"
    runner.invoke(app, ["db", "add", str(minifam_db), "1"])
    consensus = files_path / "consensus.fna"
    runner.invoke(app, ["scan", "add", str(consensus), "1"])
    assert runner.invoke(app, ["scan", "rm", "1"]).exit_code == 0
