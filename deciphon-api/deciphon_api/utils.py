__all__ = ["isint"]


def isint(value: str) -> bool:
    try:
        int(value)
    except ValueError:
        return False
    return True
