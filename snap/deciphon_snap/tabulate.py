from itertools import zip_longest

__all__ = ["tabulate"]


def tabulate(tabular_data, aligns):
    cols = list(zip_longest(*tabular_data))
    cols = [["" if v is None else str(v) for v in c] for c in cols]
    cols = [_align_column(c, a) for c, a in zip(cols, aligns)]
    rows = list(zip(*cols))
    return _format_table(rows)


def _padleft(width, s):
    fmt = "{0:>%ds}" % width
    return fmt.format(s)


def _padright(width, s):
    fmt = "{0:<%ds}" % width
    return fmt.format(s)


def _align_column_choose_padfn(strings, alignment):
    strings = [s.strip() for s in strings]
    padfn = _padleft if alignment == "right" else _padright
    return strings, padfn


def _align_column(strings, alignment):
    """[string] -> [padded_string]"""
    strings, padfn = _align_column_choose_padfn(strings, alignment)

    s_widths = list(map(len, strings))
    maxwidth = max(s_widths)
    return [padfn(maxwidth, s) for s in strings]


def _build_row(padded_cells):
    "Return a string which represents a row of data cells."
    return " ".join(padded_cells).rstrip()


def _append_basic_row(lines, padded_cells):
    lines.append(_build_row(padded_cells))
    return lines


def _format_table(rows):
    """Produce a plain-text representation of the table."""
    lines = []

    padded_rows = [[cell for cell in row] for row in rows]

    for row in padded_rows:
        _append_basic_row(lines, row)

    return "\n".join(lines)
