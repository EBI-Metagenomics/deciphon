from deciphon_core.schema import FilePath
from pydantic import TypeAdapter


def file_path(x: FilePath):
    return TypeAdapter(FilePath).validate_python(x)
