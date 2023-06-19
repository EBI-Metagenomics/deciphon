from io import StringIO
from pathlib import Path
from tempfile import NamedTemporaryFile

from deciphon_api.align import Align
from deciphon_api.hmm_path import HMMPath
from deciphon_api.hmmer_domains import read_hmmer_headers, read_hmmer_paths
from deciphon_api.hmmer_result import HMMERResult
from deciphon_api.liner import mkliner
from deciphon_api.models import Prod
from deciphon_api.snap_fs import snap_fs


def align_prod(prod: Prod):
    hmm = HMMPath.make(prod.match)
    fs = snap_fs(prod.snap.sha256)
    with NamedTemporaryFile() as tmp:
        fs.get_file(f"snap/hmmer/{prod.seq_id}/{prod.profile}.h3r", tmp.file.name)
        hmmer = HMMERResult(Path(tmp.file.name))
        headers = list(read_hmmer_headers(mkliner(data=StringIO(hmmer.domains()))))
        hmmers = read_hmmer_paths(mkliner(data=StringIO(hmmer.domains())))
        return Align(hmm, headers, hmmers)
