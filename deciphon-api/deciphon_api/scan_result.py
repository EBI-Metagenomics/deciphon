from __future__ import annotations

import dataclasses
import io
from typing import List

from Bio import SeqIO
from Bio.Seq import Seq as BioSeq
from Bio.SeqFeature import FeatureLocation, SeqFeature
from Bio.SeqRecord import SeqRecord

from deciphon_api.gff import GFFWriter
from deciphon_api.models import Prod, Scan, Seq

EPSILON = "0.01"

__all__ = ["ScanResult"]


@dataclasses.dataclass
class Match:
    state: str
    query: str
    codon: str
    amino: str

    def get(self, field: str):
        return dataclasses.asdict(self)[field]


@dataclasses.dataclass
class Hit:
    id: int
    name: str
    prod_id: int
    evalue: float
    matchs: List[Match] = dataclasses.field(default_factory=lambda: [])
    feature_start: int = 0
    feature_end: int = 0


def is_core_state(state: str):
    return state.startswith("M") or state.startswith("I") or state.startswith("D")


class ScanResult:
    scan: Scan
    hits: List[Hit]

    def __init__(self, scan: Scan):
        self.scan = scan
        self.hits: List[Hit] = []

        for prod in self.scan.prods:
            self._make_hits(prod)

    def get_seq(self, seq_id: int) -> Seq:
        for seq in self.scan.seqs:
            if seq.id == seq_id:
                return seq
        assert False

    def get_prod(self, prod_id: int) -> Prod:
        for prod in self.scan.prods:
            if prod.id == prod_id:
                return prod
        assert False

    def path(self, prod: Prod):
        state_stream = []
        amino_stream = []
        codon1_stream = []
        codon2_stream = []
        codon3_stream = []
        query1_stream = []
        query2_stream = []
        query3_stream = []
        query4_stream = []
        query5_stream = []
        for query_match in prod.match.split(";"):
            query, state, codon, amino = query_match.split(",")
            state_stream.append(state[0])
            amino_stream.append(amino)
            codon1_stream.append(codon[0] if len(codon) > 0 else " ")
            codon2_stream.append(codon[1] if len(codon) > 1 else " ")
            codon3_stream.append(codon[2] if len(codon) > 2 else " ")
            query1_stream.append(query[0] if len(query) > 0 else " ")
            query2_stream.append(query[1] if len(query) > 1 else " ")
            query3_stream.append(query[2] if len(query) > 2 else " ")
            query4_stream.append(query[3] if len(query) > 3 else " ")
            query5_stream.append(query[4] if len(query) > 4 else " ")
        return (
            "".join(state_stream),
            "".join(amino_stream),
            "".join(codon1_stream),
            "".join(codon2_stream),
            "".join(codon3_stream),
            "".join(query1_stream),
            "".join(query2_stream),
            "".join(query3_stream),
            "".join(query4_stream),
            "".join(query5_stream),
        )

    def _make_hits(self, prod: Prod):
        hit_start = 0
        hit_end = 0
        offset = 0
        hit_start_found = False
        hit_end_found = False

        for query_match in prod.match.split(";"):
            query, state, codon, amino = query_match.split(",")

            if not hit_start_found and is_core_state(state):
                hit_start = offset
                hit_start_found = True
                evalue = prod.evalue
                name = self.get_seq(prod.seq_id).name
                self.hits.append(Hit(len(self.hits) + 1, name, prod.id, evalue))

            if hit_start_found and not is_core_state(state):
                hit_end = offset + len(query)
                hit_end_found = True

            if hit_start_found and not hit_end_found:
                self.hits[-1].matchs.append(Match(state[0], query, codon, amino))

            if hit_end_found:
                self.hits[-1].feature_start = hit_start
                self.hits[-1].feature_end = hit_end
                hit_start_found = False
                hit_end_found = False

            offset += len(query)

    def gff(self):
        if len(self.scan.prods) == 0:
            return "##gff-version 3\n"

        gff = GFFWriter()

        for prod in self.scan.prods:
            hits = [hit for hit in self.hits if hit.prod_id == prod.id]

            seq = BioSeq(self.get_seq(prod.seq_id).data)
            rec = SeqRecord(seq, self.get_seq(prod.seq_id).name)

            evalue = hits[0].evalue
            qualifiers = {
                "source": f"deciphon:{prod.version}",
                "score": f"{evalue:.17g}",
                "Target_alph": prod.abc,
                "Profile_acc": prod.profile,
                "Epsilon": EPSILON,
            }

            for hit in hits:
                feat = SeqFeature(
                    FeatureLocation(hit.feature_start, hit.feature_end, strand=None),
                    type="CDS",
                    qualifiers=dict(qualifiers, ID=hit.id),
                )
                rec.features.append(feat)

            gff.add(rec)

        return gff.dumps()

    def fasta(self, type_):
        assert type_ in ["amino", "query", "codon", "state"]

        recs = []

        for prod in self.scan.prods:
            hits = [hit for hit in self.hits if hit.prod_id == prod.id]
            for hit in hits:
                recs.append(
                    SeqRecord(
                        BioSeq("".join([m.get(type_) for m in hit.matchs])),
                        id=str(hit.id),
                        description=hit.name,
                    )
                )

        fasta_io = io.StringIO()
        SeqIO.write(recs, fasta_io, "fasta")
        fasta_io.seek(0)
        return fasta_io.read()

    def hmmer_targets(self):
        for prod in self.scan.prods:
            prod.hmmer_sha256
        pass
        # h3result_targets
