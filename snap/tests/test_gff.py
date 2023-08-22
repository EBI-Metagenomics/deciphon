from pathlib import Path

from deciphon_snap.read_snap import read_snap

desired = """##gff-version 3
0	deciphon	CDS	454	738	9.9615623609338152e-07	+	0	Profile=PF00004.32;Alphabet=dna;ID=1
0	deciphon	CDS	451	606	1.009051514531623e-07	+	0	Profile=PF03969.19;Alphabet=dna;ID=2
"""


def test_gff(files_path: Path):
    snap_file = read_snap(files_path / "example.dcs")
    prod = snap_file.products[0:2]
    assert prod.gff_list().format() == desired
