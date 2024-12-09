import os
import pkgutil
from io import StringIO

from fasta_reader import Formatter, Reader
from xdg_base_dirs import xdg_cache_home
from xxhash import xxh64_hexdigest

RAW_SEQUENCE = """
>Homoserine_dh-consensus
CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGG
ATATTAAACGGCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTC
TCTGAGGCGCTGAAGGAGGCACAGGAATTGGGCTACGCGGAAGCGGATCCTACGGACGAT
GTGGAAGGGCTAGATGCTGCTAGAAAGCTGGCAATTCTAGCCAGATTGGCATTTGGGTTA
GAGGTCGAGTTGGAGGACGTAGAGGTGGAAGGAATTGAAAAGCTGACTGCCGAAGATATT
GAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGTCGAAGCCAGG
GTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC
GCCGTGGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGC
GCAGAGCCAACCGCATCCGCTGTACTCGCTGACCTTCTC
"""

CACHEDIR = xdg_cache_home() / "deciphon-gui"


def example_sequence(chunk=10, width=76):
    formatter = Formatter(chunk=chunk, width=width)
    sequences: list[str] = []
    for x in Reader(StringIO(RAW_SEQUENCE)):
        sequences.append(formatter.format(x))
    return "\n".join(sequences).rstrip().split("\n")


def example_hmmfile():
    os.makedirs(CACHEDIR, exist_ok=True)
    minifam = CACHEDIR / "minifam.hmm"
    desired = "8aca59c4bc167d72"
    actual = xxh64_hexdigest(open(minifam, "rb").read()) if minifam.exists() else ""

    if desired != actual:
        data = pkgutil.get_data("deciphon_gui", "minifam.hmm")
        assert data is not None
        with open(minifam, "wb") as f:
            f.write(data)

    return minifam
