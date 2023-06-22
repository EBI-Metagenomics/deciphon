from deciphon_snap.interval import PyInterval
from deciphon_snap.match import MatchList, MatchListInterval

__all__ = ["QueryInterval", "QueryIntervalBuilder"]


class QueryInterval(PyInterval):
    ...


class QueryIntervalBuilder:
    def __init__(self, match_list: MatchList):
        self._offset = []
        offset = 0
        for x in match_list:
            self._offset.append(offset)
            offset += len(x.query)
        self._offset.append(offset)

    def make(self, match_list_interval: MatchListInterval) -> QueryInterval:
        i = match_list_interval
        start = self._offset[i.pyinterval.start]
        stop = self._offset[i.pyinterval.stop]
        return QueryInterval(start=start, stop=stop)
