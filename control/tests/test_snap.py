import os
from pathlib import Path

from deciphonctl.cli import app


def test_add(runner, files_path: Path):
    minifam_hmm = files_path / "minifam.hmm"
    runner.invoke(app, ["hmm", "add", str(minifam_hmm), "1"])
    minifam_db = files_path / "minifam.dcp"
    runner.invoke(app, ["db", "add", str(minifam_db), "1"])
    consensus = files_path / "consensus.fna"
    runner.invoke(app, ["scan", "add", str(consensus), "1"])
    snap = files_path / "snap.dcs"
    assert runner.invoke(app, ["scan", "snap-add", "1", str(snap)]).exit_code == 0


def test_get(runner, files_path: Path):
    minifam_hmm = files_path / "minifam.hmm"
    runner.invoke(app, ["hmm", "add", str(minifam_hmm), "1"])
    minifam_db = files_path / "minifam.dcp"
    runner.invoke(app, ["db", "add", str(minifam_db), "1"])
    consensus = files_path / "consensus.fna"
    runner.invoke(app, ["scan", "add", str(consensus), "1"])
    snap = files_path / "snap.dcs"
    runner.invoke(app, ["scan", "snap-add", "1", str(snap)])
    assert runner.invoke(app, ["scan", "snap-get", "1"]).exit_code == 0
    os.unlink("snap.dcs")


def test_rm(runner, files_path: Path):
    minifam_hmm = files_path / "minifam.hmm"
    runner.invoke(app, ["hmm", "add", str(minifam_hmm), "1"])
    minifam_db = files_path / "minifam.dcp"
    runner.invoke(app, ["db", "add", str(minifam_db), "1"])
    consensus = files_path / "consensus.fna"
    runner.invoke(app, ["scan", "add", str(consensus), "1"])
    snap = files_path / "snap.dcs"
    runner.invoke(app, ["scan", "snap-add", "1", str(snap)])
    assert runner.invoke(app, ["scan", "snap-rm", "1"]).exit_code == 0
