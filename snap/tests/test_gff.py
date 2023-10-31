from pathlib import Path

from deciphon_snap.read_snap import read_snap

desired = """##gff-version 3
2	deciphon	CDS	76	546	1.3e-37	+	0	Profile=PF00005.30;Alphabet=dna;ID=1
17	deciphon	CDS	130	270	1.3e-20	+	0	Profile=PF00005.30;Alphabet=dna;ID=2
17	deciphon	CDS	439	522	1.3e-20	+	0	Profile=PF00005.30;Alphabet=dna;ID=3
"""


def test_gff(files_path: Path):
    snap_file = read_snap(files_path / "example.dcs")
    prod = snap_file.products[0:2]
    assert prod.gff_list().format() == desired


def test_empty_gff(files_path: Path):
    snap_file = read_snap(files_path / "example.dcs")
    prod = snap_file.products[0:0]
    assert prod.gff_list().format() == "##gff-version 3\n"
