from deciphon_core.batch import Batch
from deciphon_core.sequence import Sequence


def create_batch(sequences: list[Sequence]):
    batch = Batch()
    for seq in sequences:
        batch.add(seq)
    return batch
