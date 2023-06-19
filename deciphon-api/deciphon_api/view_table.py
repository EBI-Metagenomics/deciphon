import tabulate

from deciphon_api.align_prod import align_prod
from deciphon_api.models import Prod
from deciphon_api.render import Viewport


def view_table(prod: Prod):
    seqid = prod.seq.name
    align = align_prod(prod)
    profile = prod.profile
    seqid = prod.seq.name
    viewport = Viewport(align.coord, ".")
    txt = "Alignments for each domain:\n"
    for hdr, hit in zip(align.headers, align.hits):
        table = []
        COLS = 88
        intervals = list(hit.interval.intervals(size=COLS))
        width = intervals[0].length
        txt += hdr + "\n"
        for i in intervals:
            v = viewport.cut(i)
            h = hit.cut(i)
            amino = [y for x in h.hmm if (y := x.amino())]
            hmm_cs = [x.hmm_cs() for x in h.hmmer]
            match = [x.match() for x in h.hmmer]
            h3query = [x.query() for x in h.hmmer]
            score = [x.score() for x in h.hmmer]
            q0 = [y for x in h.hmm if (y := x.query(0))]
            q1 = [y for x in h.hmm if (y := x.query(1))]
            q2 = [y for x in h.hmm if (y := x.query(2))]
            q3 = [y for x in h.hmm if (y := x.query(3))]
            q4 = [y for x in h.hmm if (y := x.query(4))]
            profile_left, profile_right = h.state_bounds
            query_left, query_right = h.query_bounds
            amino_left, amino_right = h.amino_bounds
            pad = "&" * (width - i.length)
            row = [
                [None, None, v.display(hmm_cs) + pad, "CS"],
                [profile, profile_left, v.display(h3query) + pad, profile_right],
                [None, None, v.display(match) + pad, None],
                [seqid, amino_left, v.display(amino) + pad, amino_right],
                [None, query_left, v.display(q0) + pad, query_right],
                [None, None, v.display(q1) + pad, None],
                [None, None, v.display(q2) + pad, None],
                [None, None, v.display(q3) + pad, None],
                [None, None, v.display(q4) + pad, None],
                [None, None, v.display(score) + pad, "PP"],
            ]
            table += row + [[None, None, None]]
        tablefmt = tabulate.simple_separated_format(" ")
        txt += str(
            tabulate.tabulate(
                table, tablefmt=tablefmt, colalign=("right", "right", "left", "left")
            )
        )
        txt = txt.replace("&", "") + "\n"
    return txt
