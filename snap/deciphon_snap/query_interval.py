from deciphon_intervals import PyInterval

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
            offset += x.query_size
        self._offset.append(offset)

    def make(self, match_list_interval: MatchListInterval) -> QueryInterval:
        i = match_list_interval
        start = self._offset[i.py.start]
        stop = self._offset[i.py.stop]
        return QueryInterval(start=start, stop=stop)
