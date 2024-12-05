from __future__ import annotations

import hashlib
import os
import shutil
from pathlib import Path

import h3daemon
from deciphon_schema import DBFile, Gencode, HMMFile, NewSnapFile

from deciphon_core.batch import Batch
from deciphon_core.press import PressContext
from deciphon_core.scan import Scan
from deciphon_core.sequence import Sequence


def checksum(filename: Path):
    hash_obj = hashlib.md5()
    with open(filename, "rb") as file:
        for chunk in iter(lambda: file.read(4096), b""):
            hash_obj.update(chunk)
    return hash_obj.hexdigest()


def run_scan(hmm: Path, hmmfile: HMMFile, num_threads: int, cache: bool):
    shutil.rmtree("snap.dcs", ignore_errors=True)
    shutil.rmtree("snap", ignore_errors=True)
    snapfile = NewSnapFile(path=Path("snap.dcs").absolute())

    with h3daemon.daemon_context(hmmfile) as daemon:
        dbfile = DBFile(path=hmmfile.dbpath.path)
        batch = Batch()
        for seq in sequences:
            batch.add(seq)
        scan = Scan(dbfile, daemon.port(), num_threads, True, False, False)
        with scan:
            scan.run(snapfile, batch)
            snapfile.make_archive()
            assert scan.progress() == 100

    shutil.unpack_archive(snapfile.path, format="zip")
    products = snapfile.basedir / "products.tsv"
    assert checksum(products)[:8] == "a8d59263"


sequences = [
    Sequence(
        1,
        "Homoserine_dh-consensus",
        "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
        "GCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTCTCTGAGGCGCTGAAGGAGGC"
        "ACAGGAATTGGGCTACGCGGAAGCGGATCCTACGGACGATGTGGAAGGGCTAGATGCTGCTAGAAAGCTG"
        "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGAGGTGGAAGGAATTGAAA"
        "AGCTGACTGCCGAAGATATTGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGT"
        "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC"
        "GCCGTGGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
        "CCGCATCCGCTGTACTCGCTGACCTTCTC",
    ),
    Sequence(
        2,
        "AA_kinase-consensus",
        "AAACGTGTAGTTGTAAAGCTTGGGGGTAGTTCTCTGACAGATAAGGAAGAGGCATCACTCAGGCGTTTAG"
        "CTGAGCAGATTGCAGCATTAAAAGAGAGTGGCAATAAACTAGTGGTCGTGCATGGAGGCGGCAGCTTCAC"
        "TGATGGTCTGCTGGCATTGAAAAGTGGCCTGAGCTCGGGCGAATTAGCTGCGGGGTTGAGGAGCACGTTA"
        "GAAGAGGCCGGAGAAGTAGCGACGAGGGACGCCCTAGCTAGCTTAGGGGAACGGCTTGTTGCAGCGCTGC"
        "TGGCGGCGGGTCTCCCTGCTGTAGGACTCAGCGCCGCTGCGTTAGATGCGACGGAGGCGGGCCGGGATGA"
        "AGGCAGCGACGGGAACGTCGAGTCCGTGGACGCAGAAGCAATTGAGGAGTTGCTTGAGGCCGGGGTGGTC"
        "CCCGTCCTAACAGGATTTATCGGCTTAGACGAAGAAGGGGAACTGGGAAGGGGATCTTCTGACACCATCG"
        "CTGCGTTACTCGCTGAAGCTTTAGGCGCGGACAAACTCATAATACTGACCGACGTAGACGGCGTTTACGA"
        "TGCCGACCCTAAAAAGGTCCCAGACGCGAGGCTCTTGCCAGAGATAAGTGTGGACGAGGCCGAGGAAAGC"
        "GCCTCCGAATTAGCGACCGGTGGGATGAAGGTCAAACATCCAGCGGCTCTTGCTGCAGCTAGACGGGGGG"
        "GTATTCCGGTCGTGATAACGAAT",
    ),
    Sequence(
        3,
        "23ISL-consensus",
        "CAGGGTCTGGATAACGCTAATCGTTCGCTAGTTCGCGCTACAAAAGCAGAAAGTTCAGATATACGGAAAG"
        "AGGTGACTAACGGCATCGCTAAAGGGCTGAAGCTAGACAGTCTGGAAACAGCTGCAGAGTCGAAGAACTG"
        "CTCAAGCGCACAGAAAGGCGGATCGCTAGCTTGGGCAACCAACTCCCAACCACAGCCTCTCCGTGAAAGT"
        "AAGCTTGAGCCATTGGAAGACTCCCCACGTAAGGCTTTAAAAACACCTGTGTTGCAAAAGACATCCAGTA"
        "CCATAACTTTACAAGCAGTCAAGGTTCAACCTGAACCCCGCGCTCCCGTCTCCGGGGCGCTGTCCCCGAG"
        "CGGGGAGGAACGCAAGCGCCCAGCTGCGTCTGCTCCCGCTACCTTACCGACACGACAGAGTGGTCTAGGT"
        "TCTCAGGAAGTCGTTTCGAAGGTGGCGACTCGCAAAATTCCAATGGAGTCACAACGCGAGTCGACT",
    ),
    Sequence(
        4,
        "Homoserine_dh-consensus-error",
        "TATCATTTCGATTGTCAAGGAGTCGCTGACAGGTNNNNNNNNNNNNNNNNNCGAAGGGATATTAAACGCT"
        "ACAGGAATTGGGCGACGCGGAAGCGGATCCTACGGACGATGTGGAAGGGCTAGATGCTGCTAGAAAGCTG"
        "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGAGGTGGAAGGAATTGAAA"
        "AGCTGACTGCCGAGGATATTGAAGAAGCGAAGGAAGAGTTTTAAAACTAGTGGCAAGCGCCGTCGAAGCC"
        "CTGTAGAAACGGAGCGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAACCGCATC"
        "CCATACGCTGCTGACCTTCTC",
    ),
]


def test_scan(tmp_path, files_path: Path):
    shutil.copy(files_path / "minifam.hmm", tmp_path)
    os.chdir(tmp_path)

    hmm = Path("minifam.hmm").resolve()
    with PressContext(HMMFile(path=hmm), Gencode(1)) as press:
        while not press.end():
            press.next()

    hmmfile = HMMFile(path=hmm)

    run_scan(hmm, hmmfile, 1, False)
    run_scan(hmm, hmmfile, 1, True)
    run_scan(hmm, hmmfile, 2, False)
    run_scan(hmm, hmmfile, 2, True)
