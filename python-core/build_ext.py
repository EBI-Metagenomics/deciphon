import os
import shutil
from pathlib import Path
from subprocess import check_call

from cffi import FFI
from git import Repo

CWD = Path(".").resolve()
TMP = CWD / ".build_ext"
PKG = CWD / "deciphon_core"


def make(
    cwd: Path,
    args: list[str] = [],
):
    check_call(["make"] + args, cwd=cwd)


def build_and_install(root: Path, prefix: str, prj_dir: str, git_url: str):
    git_dir = git_url.split("/")[-1].split(".")[-1][:-4]
    if not (root / git_dir).exists():
        Repo.clone_from(git_url, root / git_dir, depth=1)
    bld_dir = root / prj_dir
    args = [
        f"C_INCLUDE_PATH={prefix}/include",
        f"LIBRARY_PATH={prefix}/lib",
        "CFLAGS=-std=c11 -O3 -fPIC",
    ]
    make(bld_dir, args)
    make(bld_dir, ["install", f"PREFIX={prefix}"])


if __name__ == "__main__":
    shutil.rmtree(TMP, ignore_errors=True)
    os.makedirs(TMP, exist_ok=True)

    url = "https://github.com/EBI-Metagenomics/lite-pack.git"
    build_and_install(TMP / "lite-pack", str(PKG), ".", url)

    url = "https://github.com/EBI-Metagenomics/lite-pack.git"
    build_and_install(TMP / "lite-pack", str(PKG), "ext/", url)

    url = "https://github.com/EBI-Metagenomics/imm.git"
    build_and_install(TMP / "imm", str(PKG), ".", url)

    url = "https://github.com/EBI-Metagenomics/hmmer3.git"
    build_and_install(TMP / "hmmer3", str(PKG), "hmmer-reader/", url)

    url = "https://github.com/EBI-Metagenomics/hmmer3.git"
    build_and_install(TMP / "hmmer3", str(PKG), "h3result/", url)

    url = "https://github.com/EBI-Metagenomics/hmmer3.git"
    build_and_install(TMP / "hmmer3", str(PKG), "h3client/", url)

    url = "https://github.com/EBI-Metagenomics/deciphon.git"
    build_and_install(TMP / "deciphon", str(PKG), "c-core/", url)

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
        ],
        library_dirs=[str(PKG / "lib")],
        include_dirs=[str(PKG / "include")],
        extra_compile_args=["-fopenmp"] if os.uname().sysname == "Linux" else [],
    )

    ffibuilder.compile(verbose=True)
