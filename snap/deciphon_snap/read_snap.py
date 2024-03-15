from fsspec.implementations.cached import SimpleCacheFileSystem
from fsspec.implementations.memory import MemoryFile
from fsspec.implementations.zip import ZipFileSystem

from deciphon_snap.path_like import PathLike
from deciphon_snap.snap_file import SnapFile

__all__ = ["read_snap"]


def read_snap(filename: PathLike):
    fo = filename.decode() if isinstance(filename, bytes) else str(filename)
    with open(fo, "rb") as f:
        fs = ZipFileSystem(fo=MemoryFile(data=f.read()))
        return SnapFile(SimpleCacheFileSystem(fs=fs))
