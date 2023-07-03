import re
from dataclasses import dataclass
from itertools import chain

import portion

from deciphon_eval.portion_utils import interval_size

__all__ = ["Location"]


repr_regex: re.Pattern = re.compile(r"^Location\((.*)\)$")


@dataclass
class Location:
    intervals: list[portion.Interval]

    @classmethod
    def from_string(cls, raw: str):
        match = repr_regex.match(raw)
        assert match is not None
        intervals = [_make_interval(x) for x in match.group(1).split(",")]
        return Location(intervals=intervals)

    @classmethod
    def from_defline(cls, defline: str):
        x = []
        for i in _normalise(_grab_location_field(defline)).split(","):
            x.append(_make_interval(i))
        return Location(intervals=x)

    def __repr__(self):
        x = [str(x).lstrip("[").rstrip("]").replace(",", "..") for x in self.intervals]
        y = ",".join(x)
        return f"Location({y})".replace(" ", "")

    def __str__(self):
        return repr(self)

    def project(self, index: int):
        loc = chain.from_iterable(portion.iterate(x, step=1) for x in self.intervals)
        return int(list(loc)[index])

    def count(self):
        return sum(interval_size(x) for x in self.intervals)


def _normalise(loc: str):
    # Don't care if the sequence is partion on 5' or 3' ends
    loc = loc.replace("<", "").replace(">", "")

    # Don't care from which strand it came from
    if g := re.match(r"complement\((.*)\)", loc):
        loc = g.group(1)

    # Lets have everything in the format: join(start..stop,start..stop)
    if g := re.match(r"^(\d+)\.\.(\d+)$", loc):
        loc = f"join({g.group(1)}..{g.group(2)})"

    # Remove join operator
    g = re.match(r"^join\(([\d|.|,]*)\)$", loc)
    assert g
    return g.group(1)


def _grab_location_field(line: str):
    g = re.search(r"^.*\[location=([^\]]*)\].*$", line)
    assert g
    return g.group(1)


def _make_interval(x: str):
    if ".." in x:
        lower, upper = x.split("..")
    else:
        lower = upper = x
    return portion.closed(int(lower), int(upper))


if __name__ == "__main__":
    print(Location.from_defline("[location=11..22]"))
    print(Location.from_defline("[location=join(172687..172755,172757..173794)]"))
    print(Location.from_defline("[location=join(7432..7966,1..62)]"))

    x = Location.from_defline("[location=join(7432..7966,1..62)]")
    y = Location.from_string(str(x))
    assert x.intervals == y.intervals

    print(x)
    for i in range(x.count()):
        print(x.project(i))
