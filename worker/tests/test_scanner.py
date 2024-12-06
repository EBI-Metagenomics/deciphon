import os
import shutil
from pathlib import Path

from deciphon_core.scan import NewSnapFile
from deciphon_core.sequence import Sequence
from deciphon_schema import Gencode, HMMFile

from deciphon_worker import launch_scanner, press

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


def test_scan_worker_1(tmp_path, files_path: Path):
    os.chdir(tmp_path)
    shutil.copy(files_path / "minifam.hmm", Path("minifam.hmm"))
    hmmfile = HMMFile(path=Path("minifam.hmm"))
    task = press(hmmfile, Gencode.BAPP, 0.01)
    dbfile = task.result()
    assert os.path.getsize(dbfile.path) == 3609858

    scanner = launch_scanner(task.result()).result()
    with scanner:
        pass


def test_scan_worker_2(tmp_path, files_path: Path):
    os.chdir(tmp_path)
    shutil.copy(files_path / "minifam.hmm", Path("minifam.hmm"))
    hmmfile = HMMFile(path=Path("minifam.hmm"))
    task = press(hmmfile, Gencode.BAPP, 0.01)
    dbfile = task.result()
    assert os.path.getsize(dbfile.path) == 3609858

    scanner = launch_scanner(dbfile).result()
    with scanner:
        task = scanner.put(NewSnapFile(path=Path("result.dcs")), sequences)
        task.result()
        assert task.done
        assert task.progress == 100


def test_scan_worker_3(tmp_path, files_path: Path):
    os.chdir(tmp_path)
    shutil.copy(files_path / "minifam.hmm", Path("minifam.hmm"))
    hmmfile = HMMFile(path=Path("minifam.hmm"))
    task = press(hmmfile, Gencode.BAPP, 0.01)
    dbfile = task.result()
    assert os.path.getsize(dbfile.path) == 3609858

    scanner = launch_scanner(dbfile).result()
    with scanner:
        products = [
            scanner.put(NewSnapFile(path=Path("result.1.dcs")), sequences),
            scanner.put(NewSnapFile(path=Path("result.2.dcs")), sequences),
            scanner.put(NewSnapFile(path=Path("result.3.dcs")), sequences),
        ]
        for x in products:
            x.result()
            assert x.done
            assert x.progress == 100

    scanner = launch_scanner(dbfile).result()
    with scanner:
        products = [
            scanner.put(NewSnapFile(path=Path("result.4.dcs")), sequences),
            scanner.put(NewSnapFile(path=Path("result.5.dcs")), sequences),
            scanner.put(NewSnapFile(path=Path("result.6.dcs")), sequences),
            scanner.put(NewSnapFile(path=Path("result.7.dcs")), sequences),
            scanner.put(NewSnapFile(path=Path("result.8.dcs")), sequences),
        ]

    scanner = launch_scanner(dbfile).result()
    with scanner:
        products = [
            scanner.put(NewSnapFile(path=Path("result.9.dcs")), sequences),
            scanner.put(NewSnapFile(path=Path("result.10.dcs")), sequences),
            scanner.put(NewSnapFile(path=Path("result.11.dcs")), sequences),
        ]
        for x in products:
            x.result()
            assert x.done
            assert x.progress == 100


def test_scan_worker_4(tmp_path, files_path: Path):
    os.chdir(tmp_path)
    shutil.copy(files_path / "minifam.hmm", Path("minifam.hmm"))
    hmmfile = HMMFile(path=Path("minifam.hmm"))

    task = press(hmmfile, Gencode.BAPP, 0.01)
    dbfile = task.result()
    assert os.path.getsize(dbfile.path) == 3609858

    scanner = launch_scanner(dbfile).result()
    with scanner:
        task = scanner.put(NewSnapFile(path=Path("result.1.dcs")), sequences)
        for i in task.as_progress():
            pass
        assert task.done
        assert task.progress == 100
