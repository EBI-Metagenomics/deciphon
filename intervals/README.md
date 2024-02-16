# deciphon-intervals

It helps to model the two primary interval definitions in the programming
realm: 0-start, half-open interval (aka Python interval), and 1-start,
fully-closed interval (aka R interval).

## Example

```python
from deciphon_intervals import PyInterval, RInterval, Interval


x = [1, 2, 4, 8]
print(x[PyInterval(start=1, stop=3).slice])
print(x[RInterval(start=2, stop=3).slice])
# [2, 4]
# [2, 4]

interval: Interval = RInterval(start=2, stop=3)

print(interval.py)
print(interval.r)
# PyInterval(start=1, stop=3)
# RInterval(start=2, stop=3)
```
