import tempfile
from pathlib import Path
from subprocess import check_output

from deciphon_api.config import get_config

__all__ = ["HMMERResult"]


class HMMERResult:
    def __init__(self, filepath: Path):
        self._filepath = filepath
        self._bin = get_config().h3result

    def targets(self) -> str:
        output = check_output([self._bin, "--targets", str(self._filepath)])
        return output.decode()

    def domains(self) -> str:
        output = check_output([self._bin, "--domains", str(self._filepath)])
        return output.decode()

    def targets_table(self) -> str:
        output = check_output([self._bin, "--targets-table", str(self._filepath)])
        return output.decode()

    def domains_table(self) -> str:
        output = check_output([self._bin, "--domains-table", str(self._filepath)])
        return output.decode()

    def stream(self):
        file = tempfile.NamedTemporaryFile("w")
        with open(file.name, "w") as f:
            f.write(self.domains())
        with open(file.name, "r") as f:
            return self._stream2(f)

    def hmm_cs_stream(self):
        return self.stream()[0]

    def seq_cs_stream(self):
        return self.stream()[1]

    def match_stream(self):
        return self.stream()[2]

    def target_stream(self):
        return self.stream()[3]

    def pp_stream(self):
        return self.stream()[4]

    def _stream(self, file):
        state = 0

        domain_header = []
        hmm_cs = []
        seq_cs = []
        match = []
        target = []
        post_prob = []

        for row in file:
            row = row.strip()
            if len(row) == 0:
                continue
            if state == 0 and row.replace(" ", "").startswith("=="):
                domain_header.append(row.strip())
                state += 1
            elif state == 1:
                last = row.rfind(" ")
                assert row[last:].strip() == "CS"
                hmm_cs.append(row[:last].strip())
                state += 1
            elif state == 2:
                fields = row.split()
                acc = fields[0]
                start = fields[1]
                offset = row.find(acc) + len(acc)
                offset = row.find(start, offset) + len(start)
                last = row.rfind(" ")
                seq = row[offset:last]
                end = row[last:]

                acc = acc.strip()
                start = start.strip()
                seq = seq.strip()
                end = end.strip()

                state += 1
                seq_cs.append({"acc": acc, "start": start, "seq": seq, "end": end})
            elif state == 3:
                match.append(row.strip())
                state += 1
            elif state == 4:
                fields = row.split()
                start = fields[0]
                offset = row.find(start) + len(start)
                last = row.rfind(" ")
                seq = row[offset:last]
                end = row[last:]

                start = start.strip()
                seq = seq.strip()
                end = end.strip()
                target.append({"start": start, "seq": seq, "end": end})
                state += 1
            elif state == 5:
                last = row.rfind(" ")
                assert row[last:].strip() == "PP"
                post_prob.append(row[:last].strip())
                state = 1

        hmm_cs_stream = "".join(hmm_cs)
        seq_cs_stream = "".join(x["seq"] for x in seq_cs)
        match_stream = "".join(match)
        target_stream = "".join(x["seq"] for x in target)
        pp_stream = "".join(post_prob)

        return (hmm_cs_stream, seq_cs_stream, match_stream, target_stream, pp_stream)

    def _stream2(self, file):
        state = 0

        domain_header = []
        hmm_cs = []
        seq_cs = []
        match = []
        target = []
        pp = []
        prev_end = 0

        target_start = 0
        target_end = 0

        for row in file:
            row = row.strip()
            if len(row) == 0:
                continue
            if state == 0 and row.replace(" ", "").startswith("=="):
                domain_header.append(row.strip())
                state += 1
            elif state == 1:
                last = row.rfind(" ")
                assert row[last:].strip() == "CS"
                hmm_cs.append(row[:last].strip())
                state += 1
            elif state == 2:
                fields = row.split()
                acc = fields[0]
                start = fields[1]
                offset = row.find(acc) + len(acc)
                offset = row.find(start, offset) + len(start)
                last = row.rfind(" ")
                seq = row[offset:last]
                end = row[last:]

                acc = acc.strip()
                start = start.strip()
                seq = seq.strip()
                end = end.strip()

                state += 1
                seq_cs.append(seq)
                # seq_cs.append({"acc": acc, "start": start, "seq": seq, "end": end})
            elif state == 3:
                match.append(row.strip())
                state += 1
            elif state == 4:
                fields = row.split()
                start = fields[0]
                offset = row.find(start) + len(start)
                last = row.rfind(" ")
                seq = row[offset:last]
                end = row[last:]

                start = start.strip()
                seq = seq.strip()
                end = end.strip()
                target.append(seq)
                target_start = int(start)
                target_end = int(end)
                # target.append({"start": start, "seq": seq, "end": end})
                state += 1
            elif state == 5:
                last = row.rfind(" ")
                assert row[last:].strip() == "PP"
                pp.append(row[:last].strip())
                state = 1
                start = target_start - 1
                # print(start)
                # print(prev_end)
                hmm_cs[-1] = " " * (start - prev_end) + hmm_cs[-1]
                seq_cs[-1] = " " * (start - prev_end) + seq_cs[-1]
                match[-1] = " " * (start - prev_end) + match[-1]
                target[-1] = " " * (start - prev_end) + target[-1]
                pp[-1] = " " * (start - prev_end) + pp[-1]
                prev_end = target_end

        hmm_cs_stream = "".join(hmm_cs)
        seq_cs_stream = "".join(seq_cs)
        match_stream = "".join(match)
        target_stream = "".join(target)
        pp_stream = "".join(pp)

        return (hmm_cs_stream, seq_cs_stream, match_stream, target_stream, pp_stream)
