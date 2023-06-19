from deciphon_snap.match_list import MatchList

__all__ = ["shorten"]


def shorten(x: str | int | float | MatchList, size: int = 32):
    if isinstance(x, float):
        return f"{x:.9g}"
    if isinstance(x, int):
        return str(x)
    if isinstance(x, MatchList):
        x = str(x)
    return x[:size] + "…" if len(x) > size else x
