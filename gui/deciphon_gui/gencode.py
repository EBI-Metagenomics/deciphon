from deciphon_schema import Gencode, GencodeName

from deciphon_gui.shorten import shorten


def gencode_description(code: Gencode, name: GencodeName):
    return f"{code.value}. {shorten(name.value, 46)}"
