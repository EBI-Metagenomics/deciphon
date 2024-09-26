import os
import shutil
from pathlib import Path
from subprocess import check_call

from cffi import FFI
from git import Repo


def make(cwd: Path, args: list[str] = []):
    check_call(["make"] + args, cwd=cwd)


def build_and_install(
    root: Path, prefix: str, git_url: str, prj_dir: str, dst_dir: str, force: bool
):
    os.makedirs(root / ".gitdir", exist_ok=True)
    if force or not (root / dst_dir).exists():
        shutil.rmtree(root / ".gitdir" / dst_dir, ignore_errors=True)
        Repo.clone_from(git_url, root / ".gitdir" / dst_dir, depth=1)
        shutil.rmtree(root / dst_dir, ignore_errors=True)
        shutil.move(root / ".gitdir" / dst_dir / prj_dir, root / dst_dir)

    args = [
        f"C_INCLUDE_PATH={prefix}/include",
        f"LIBRARY_PATH={prefix}/lib",
        "CFLAGS=-std=c11 -O3 -fPIC",
    ]
    make(root / dst_dir, args)
    make(root / dst_dir, ["install", f"PREFIX={prefix}"])


if __name__ == "__main__":
    CWD = Path(".").resolve()
    TMP = CWD / ".build_ext"
    PKG = CWD / "deciphon_core"

    url = "https://github.com/EBI-Metagenomics/lite-pack.git"
    build_and_install(TMP, str(PKG), url, ".", "lite-pack", True)
    build_and_install(TMP, str(PKG), url, "ext/", "lite-pack-ext", True)

    url = "https://github.com/EBI-Metagenomics/imm.git"
    build_and_install(TMP, str(PKG), url, ".", "imm", True)

    url = "https://github.com/EBI-Metagenomics/hmmer3.git"
    build_and_install(TMP, str(PKG), url, "hmmer-reader/", "hmmer-reader", True)
    build_and_install(TMP, str(PKG), url, "h3result/", "h3result", True)
    build_and_install(TMP, str(PKG), url, "h3client/", "h3client", True)

    url = "https://github.com/EBI-Metagenomics/deciphon.git"
    build_and_install(TMP, str(PKG), url, "c-core/", "c-core", True)

    ffibuilder = FFI()

    ffibuilder.cdef(open(PKG / "interface.h", "r").read())
    ffibuilder.set_source(
        "deciphon_core.cffi",
        """
        #include "deciphon.h"
        """,
        language="c",
        libraries=[
            "deciphon",
            "h3client",
            "h3result",
            "hmmer_reader",
            "imm",
            "lio",
            "lite_pack",
            "gomp" if os.uname().sysname == "Linux" else "omp",
        ],
        library_dirs=[str(PKG / "lib")],
        include_dirs=[str(PKG / "include")],
        extra_compile_args=["-fopenmp"],
    )

    ffibuilder.compile(verbose=True)
