from __future__ import annotations
import os
import hashlib
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
            "--num-threads",
            "1",
        ],
    )
    assert result.exit_code == 0
    snap = Path("snap_file.dcs")
    assert snap.exists()
    shutil.unpack_archive(snap, format="zip")
    products = basedir(snap) / "products.tsv"
    assert checksum(products)[:8] == "3d5bf4b8"


def checksum(filename: Path):
    hash_obj = hashlib.md5()
    with open(filename, "rb") as file:
        for chunk in iter(lambda: file.read(4096), b""):
            hash_obj.update(chunk)
    return hash_obj.hexdigest()


def basedir(x: Path):
    return x.parent / str(x.stem)
