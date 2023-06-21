from pathlib import Path

from deciphon.seq_file import SeqFile


def test_seqfile(files_path: Path):
    with SeqFile(files_path / "sequences.fna") as seqfile:
        seqit = iter(seqfile)

        seq = next(seqit)
        assert seq.id == 0
        assert seq.name == "Homoserine_dh-consensus"
        assert seq.data.startswith("CCTA")

        seq = next(seqit)
        assert seq.id == 1
        assert seq.name == "AA_kinase-consensus"
        assert seq.data.startswith("AAAC")

        seq = next(seqit)
        assert seq.id == 2
        assert seq.name == "23ISL-consensus"
        assert seq.data.startswith("CAGG")
