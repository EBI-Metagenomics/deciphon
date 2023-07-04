from pathlib import Path
from subprocess import check_output

__all__ = ["ProfileIndex"]


class ProfileIndex:
    def __init__(self, dbhmm: Path):
        profiles = [
            x.strip().decode()
            for x in check_output(
                "grep 'ACC  '  " + str(dbhmm) + " | awk '{print $2}'", shell=True
            )
            .strip()
            .split()
        ]
        self._index = {}
        for i, x in enumerate(sorted(set(profiles))):
            self._index[x] = i

    @property
    def count(self):
        return len(self._index)

    def index(self, profile: str):
        return self._index[profile]

    def keys(self):
        return self._index.keys()
