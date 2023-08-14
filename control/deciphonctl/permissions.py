import os
from pathlib import Path


def normalise_file_permissions(file: Path):
    os.chmod(file, 0o640)
