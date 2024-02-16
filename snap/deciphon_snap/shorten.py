from deciphon_snap.hmmer import H3Result
from deciphon_snap.match import LazyMatchList, MatchList

__all__ = ["shorten"]


def shorten(
    x: str | int | float | LazyMatchList | MatchList | H3Result, size: int = 32
):
    if isinstance(x, float):
        return f"{x:.9g}"
    if isinstance(x, int):
        return str(x)
    if isinstance(x, LazyMatchList):
        x = x.evaluate()
    if isinstance(x, MatchList):
        x = str(x)
    if isinstance(x, H3Result):
        x = str(x)
    assert isinstance(x, str)
    return x[:size] + "…" if len(x) > size else x
