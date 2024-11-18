from pathlib import Path

from deciphon_snap.read_snap import read_snap
from deciphon_snap.view import view_alignments


def test_alignments(files_path: Path):
    snap_file = read_snap(files_path / "consensus.dcs")
    print("\n".join(view_alignments(snap_file)))
    print(snap_file)
