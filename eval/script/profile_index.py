from subprocess import check_output

from index import Index

__all__ = ["ProfileIndex"]


class ProfileIndex(Index):
    def __init__(self, dbhmm: str):
        profiles = [
            x.strip().decode()
            for x in check_output(
                "grep 'ACC  '  " + dbhmm + " | awk '{print $2}'", shell=True
            )
            .strip()
            .split()
        ]
        self._index = {x: i for i, x in enumerate(sorted(set(profiles)))}

    @property
    def count(self):
        return len(self._index)

    def index(self, profile: str):
        return self._index[profile]

    def keys(self):
        return self._index.keys()
