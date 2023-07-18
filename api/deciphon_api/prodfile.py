import os
import tarfile

import pydantic
from fsspec.implementations.libarchive import LibArchiveFileSystem

from deciphon_api.bufsize import BUFSIZE
from deciphon_api.models import Match
from deciphon_api.utils import isint

PathLike = str | bytes | os.PathLike


__all__ = ["ProdFileReader", "HMMERFile", "MatchFile"]


class HMMERFile:
    def __init__(self, fileobj):
        self._fileobj = fileobj

    @property
    def content(self):
        self._fileobj.seek(0)
        while content := self._fileobj.read(BUFSIZE):
            yield content


class MatchFile:
    fields = [i for i in Match.__fields__.keys() if i != "id"]

    def __init__(self, fileobj):
        self._fileobj = fileobj

        self._fileobj.seek(0)
        self._check_header()
        self._record_offset = self._fileobj.tell()

    def _check_header(self):
        # TODO: finish this
        header = self._fileobj.readline().strip()
        assert header
        # if header.split("\t") != MatchFile.fields:
        #     raise ValueError("Invalid match file header")

    def read_records(self):
        self._rewind()
        for line in self._fileobj.readlines():
            names = MatchFile.fields
            values = str(line).strip().split("\t")
            yield pydantic.parse_obj_as(Match, dict(zip(names, values)))

    def _rewind(self):
        self._fileobj.seek(self._record_offset)


class ProdFileReader:
    def __init__(self, name: PathLike):
        self._tar = tarfile.open(name, mode="r:gz")
        self._fs = LibArchiveFileSystem(str(name), compression="gzip")
        self._check_names()
        self._match_file = MatchFile(self._fs.open("prod/match.tsv", "r"))
        self._hmmer_names = [str(n) for n in self._fs.glob("prod/hmmer/*/*.h3r")]

    def match_file(self):
        return self._match_file

    def hmmer_file(self, seq_id: int, profile: str):
        for name in self._hmmer_names:
            _id = int(name.split("/", 3)[2])
            _prof = name.split("/", 3)[3][:-4]
            if _id == seq_id and _prof == profile:
                return HMMERFile(self._fs.open(name, "rb"))
        return None

    def hmmer_blob(self, seq_id: int, profile: str):
        for name in self._hmmer_names:
            _id = int(name.split("/", 3)[2])
            _prof = name.split("/", 3)[3][:-4]
            if _id == seq_id and _prof == profile:
                content = self._fs.open(name, "rb").read()
                assert isinstance(content, bytes)
                return content
        return None

    def _check_names(self):
        members = list(self._tar.getmembers())

        dirs = [i.name for i in members if i.isdir()]
        files = [i.name for i in members if i.isfile()]

        try:
            dirs.remove("prod")
        except ValueError:
            raise ValueError("prod directory not found")

        try:
            files.remove("prod/match.tsv")
        except ValueError:
            raise ValueError("match.tsv file not found")

        try:
            dirs.remove("prod/hmmer")
        except ValueError:
            pass

        for dirname in dirs:
            dirsplit = dirname.split("/", 2)
            ok = len(dirsplit) == 3
            ok = ok and dirsplit[0] == "prod"
            ok = ok and dirsplit[1] == "hmmer"
            ok = ok and isint(dirsplit[2])

            if not ok:
                raise ValueError(f"{dirname} path is not valid")

        del dirs

        for file in files:
            filesplit = file.split("/", 3)
            ok = len(filesplit) == 4
            ok = ok and filesplit[0] == "prod"
            ok = ok and filesplit[1] == "hmmer"
            ok = ok and isint(filesplit[2])
            ok = ok and len(filesplit[3]) > 4
            ok = ok and filesplit[3].endswith(".h3r")
            if not ok:
                raise ValueError(f"{file} path is not valid")
