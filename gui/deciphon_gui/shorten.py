def shorten(x: str, n: int):
    return x if len(x) <= n else (x[: n - 3] + "...")
