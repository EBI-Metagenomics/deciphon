from Bio.Seq import Seq as BioSeq
from Bio.SeqFeature import FeatureLocation, SeqFeature
from Bio.SeqRecord import SeqRecord

from deciphon_api.gff_writer import GFFWriter
from deciphon_api.make_hits import make_hits
from deciphon_api.models import Snap

EPSILON = "0.01"

__all__ = ["snap_gff"]


def snap_gff(snap: Snap):
    all_hits = [i for prod in snap.prods for i in make_hits(prod)]

    if len(snap.prods) == 0:
        return "##gff-version 3\n"

    gff = GFFWriter()

    for prod in snap.prods:
        hits = [hit for hit in all_hits if hit.prod_id == prod.id]

        seq = BioSeq(prod.seq.data)
        rec = SeqRecord(seq, prod.seq.name)

        evalue = hits[0].evalue
        qualifiers = {
            "source": "deciphon",
            "score": f"{evalue:.17g}",
            "Target_alph": prod.abc,
            "Profile_acc": prod.profile,
            "Epsilon": EPSILON,
        }

        for hit in hits:
            loc = FeatureLocation(hit.feature_start, hit.feature_end, strand=None)
            feat = SeqFeature(loc, type="CDS", qualifiers=dict(qualifiers, ID=hit.id))
            rec.features.append(feat)

        gff.add(rec)

    return gff.dumps()
