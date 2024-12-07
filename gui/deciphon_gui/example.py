from io import StringIO
from pathlib import Path
from fasta_reader import Reader, Formatter

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


def example_sequence(chunk=10, width=76):
    formatter = Formatter(chunk=chunk, width=width)
    sequences: list[str] = []
    for x in Reader(StringIO(RAW_SEQUENCE)):
        sequences.append(formatter.format(x))
    return "\n".join(sequences).rstrip().split("\n")


def example_hmmfile():
    return Path("/Users/horta/Downloads/minifam.hmm")
    # return Path("/Users/horta/Downloads/Pfam-A.500.hmm")
    # return Path("/Users/horta/Downloads/Pfam-A.one_tenth.hmm")
