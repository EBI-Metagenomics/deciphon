from __future__ import annotations

import hashlib
import os
import shutil
from pathlib import Path

from h3daemon.hmmfile import HMMFile as H3File
from h3daemon.sched import SchedContext

from deciphon_core.params import Params
from deciphon_core.press import PressContext
from deciphon_core.scan import Scan
from deciphon_core.schema import Gencode, HMMFile, NewSnapFile
from deciphon_core.sequence import Sequence


def checksum(filename: Path):
    hash_obj = hashlib.md5()
    with open(filename, "rb") as file:
        for chunk in iter(lambda: file.read(4096), b""):
            hash_obj.update(chunk)
    return hash_obj.hexdigest()


def test_scan(tmp_path, files_path: Path):
    shutil.copy(files_path / "minifam.hmm", tmp_path)
    os.chdir(tmp_path)

    hmm = Path("minifam.hmm")
    with PressContext(HMMFile(path=hmm), Gencode(1)) as press:
        while not press.end():
            press.next()

    hmmfile = H3File(hmm)
    hmmfile.ensure_pressed()
    params = Params(num_threads=1, multi_hits=True, hmmer3_compat=False)
    snapfile = NewSnapFile(path=Path("snap.dcs").absolute())

    with SchedContext(hmmfile) as sched:
        sched.is_ready(True)
        dbfile = HMMFile(path=hmm).dbfile
        scan = Scan(params, dbfile)
        scan.dial(sched.get_cport())
        with scan:
            for seq in sequences:
                scan.add(seq)
            scan.run(snapfile)
            assert not scan.interrupted()
            snapfile.make_archive()
            assert scan.progress() == 100

    shutil.unpack_archive(snapfile.path, format="zip")
    products = snapfile.basename / "products.tsv"
    assert checksum(products)[:8] == "c311e4bc"


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
