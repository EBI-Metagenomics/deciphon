import portion as pt

__all__ = ["discretize", "interval_lower", "interval_upper", "interval_size"]

_INCR = 1


def _first_step(x: pt.Interval):
    return (
        pt.OPEN,
        (x.lower - _INCR if x.left is pt.CLOSED else x.lower),
        (x.upper + _INCR if x.right is pt.CLOSED else x.upper),
        pt.OPEN,
    )


def _second_step(x: pt.Interval):
    return (
        pt.CLOSED,
        (x.lower + _INCR if x.left is pt.OPEN and x.lower != -pt.inf else x.lower),
        (x.upper - _INCR if x.right is pt.OPEN and x.upper != pt.inf else x.upper),
        pt.CLOSED,
    )


def discretize(x: pt.Interval):
    """Simplifies discrete intervals."""
    return x.apply(_first_step).apply(_second_step)


def interval_lower(x: pt.Interval):
    return int(next(pt.iterate(x, step=1)))


def interval_upper(x: pt.Interval):
    return int(next(pt.iterate(x, step=-1, reverse=True)))


def interval_size(x: pt.Interval):
    assert x.atomic
    return interval_upper(x) - interval_lower(x) + 1


if __name__ == "__main__":
    a = pt.closed(0, 1)
    b = pt.closed(2, 3)
    print(a | b)
    print(discretize(a | b))
