import hmmer

from pathlib import Path
from subprocess import check_call

__all__ = ["hmmer_press"]


def hmmer_press(hmm: Path):
    check_call([str(Path(hmmer.BIN_DIR) / "hmmpress"), str(hmm)])
