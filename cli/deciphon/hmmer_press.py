import hmmer

from pathlib import Path
from subprocess import check_output

__all__ = ["hmmer_press"]


def hmmer_press(hmm: Path):
    return check_output([str(Path(hmmer.BIN_DIR) / "hmmpress"), "-f", str(hmm)])
