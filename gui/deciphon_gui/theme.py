import sys


def font_family_mono():
    if sys.platform == "darwin":
        return "Roboto Mono"
    return "DejaVuSansMono"
