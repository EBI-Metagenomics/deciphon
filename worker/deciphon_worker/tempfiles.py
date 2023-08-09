import os
import uuid
import re
from pathlib import Path


HEXSIZE = 8
HEXCHAR = "[abcdef0-9]"
REGEX = r".*\." + HEXCHAR + "{" + HEXCHAR + "}" + r"tmp\..*"


class Tempfiles:
    def cleanup(self):
        for f in glob_re(REGEX, os.listdir()):
            Path(f).unlink(missing_ok=True)

    def unique(self, file: Path):
        hex = str(uuid.uuid4().hex)[:8]
        return file.with_suffix(f".{hex}tmp{file.suffix}")


def glob_re(pattern, strings):
    return filter(re.compile(pattern).match, strings)
