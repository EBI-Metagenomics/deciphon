from __future__ import annotations

import portion as pt

from deciphon_eval.portion_utils import interval_size

__all__ = ["ConfusionMatrix", "SolutionSpace"]


class SolutionSpace:
    def __init__(self, positive: pt.Interval, negative: pt.Interval):
        self.positive = positive
        self.negative = negative
        assert len(self.positive & self.negative) == 0


class ConfusionMatrix:
    def __init__(self, P: int, N: int, TP: int, FP: int):
        self._P = P
        self._N = N
        self._TP = TP
        self._FP = FP

    @classmethod
    def make(cls, ground_truth: SolutionSpace, guess: SolutionSpace):
        assert_comparable_solution_spaces(ground_truth, guess)
        P = sum([interval_size(x) for x in ground_truth.positive])
        N = sum([interval_size(x) for x in ground_truth.negative])
        TP = sum([interval_size(x) for x in ground_truth.positive & guess.positive])
        FP = sum([interval_size(x) for x in ground_truth.negative & guess.positive])
        return cls(P=P, N=N, TP=TP, FP=FP)

    @property
    def P(self) -> int:
        """
        Number of positive samples from the ground-truth.
        """
        return self._P

    @property
    def N(self) -> int:
        """
        Number of negative samples from the ground-truth.
        """
        return self._N

    @property
    def TP(self) -> int:
        """
        Number of true positives.
        """
        return self._TP

    @property
    def FP(self) -> int:
        """
        Number of false positives.
        """
        return self._FP

    @property
    def TN(self) -> int:
        """
        Number of true negatives.
        """
        return self.N - self.FP

    @property
    def FN(self) -> int:
        """
        Number of false negatives.
        """
        return self.P - self.TP

    @property
    def sensitivity(self) -> float:
        """
        Sensitivity.
        """
        return self.TP / self.P if self.P > 0 else 1

    @property
    def true_positive_rate(self) -> float:
        """
        True positive rate. (Same as sensitivity.)
        """
        return self.sensitivity

    @property
    def recall(self) -> float:
        """
        Recall. (Same as sensitivity.)
        """
        return self.sensitivity

    @property
    def specificity(self) -> float:
        """
        Specificity.
        """
        return self.TN / self.N if self.N > 0 else 1

    @property
    def selectivity(self) -> float:
        """
        Selectivity. (Same as specificity.)
        """
        return self.specificity

    @property
    def true_negative_rate(self) -> float:
        """
        True negative rate. (Same as specificity.)
        """
        return self.specificity

    @property
    def precision(self) -> float:
        """
        Precision.
        """
        denom = self.TP + self.FP
        return self.TP / denom if denom > 0 else 1

    @property
    def positive_predictive_value(self) -> float:
        """
        Positive predictive value. (Same as precision.)
        """
        return self.precision

    @property
    def negative_predictive_value(self) -> float:
        """
        Negative predictive value.
        """
        denom = self.TN + self.FN
        return self.TN / denom if denom > 0 else 1

    @property
    def fallout(self) -> float:
        """
        Fall-out. (1 minus specificity.)
        """
        return 1 - self.specificity

    @property
    def false_positive_rate(self) -> float:
        """
        False positive rate. (Same as fall-out.)
        """
        return self.fallout

    @property
    def false_negative_rate(self) -> float:
        """
        False negative rate. (1 minus sensitivity.)
        """
        return 1 - self.sensitivity

    @property
    def miss_rate(self):
        """
        Miss rate.
        """
        return self.false_negative_rate

    @property
    def false_omission_rate(self) -> float:
        """
        False omission rate.
        """
        return 1 - self.negative_predictive_value

    @property
    def false_discovery_rate(self) -> float:
        """
        False discovery rate. (1 minus precision)
        """
        return 1 - self.precision

    @property
    def accuracy(self):
        """
        Accuracy.
        """
        denom = self._N + self._P
        return (self.TP + self.TN) / denom if denom > 0 else 1

    @property
    def f1score(self):
        """
        F1 score.
        """
        denom = 2 * self.TP + self.FP + self.FN
        return 2 * self.TP / denom if denom > 0 else 1


def assert_comparable_solution_spaces(a: SolutionSpace, b: SolutionSpace):
    assert a.positive | a.negative == b.positive | b.negative
