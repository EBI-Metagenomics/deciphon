from io import StringIO
from typing import List

from Bio import SeqIO
from Bio.Seq import Seq as BioSeq
from Bio.SeqRecord import SeqRecord

from deciphon_api.make_hits import make_hits
from deciphon_api.models import Prod

__all__ = ["create_fasta"]


def create_fasta(prods: List[Prod], match_field: str):
    all_hits = [i for prod in prods for i in make_hits(prod)]

    assert match_field in ["amino", "query", "codon", "state"]

    recs = []

    for prod in prods:
        hits = [hit for hit in all_hits if hit.prod_id == prod.id]
        for hit in hits:
            bioseq = BioSeq("".join([m.get(match_field) for m in hit.matchs]))
            recs.append(SeqRecord(bioseq, id=str(hit.id), description=hit.name))

    fasta_io = StringIO()
    SeqIO.write(recs, fasta_io, "fasta")
    fasta_io.seek(0)
    return fasta_io.read()
