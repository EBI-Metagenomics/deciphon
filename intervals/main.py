from deciphon_intervals import PyInterval, RInterval, Interval

x = [1, 2, 4, 8]
print(x[PyInterval(start=1, stop=3).slice])
print(x[RInterval(start=2, stop=3).slice])

interval: Interval = RInterval(start=2, stop=3)

print(interval.py)
print(interval.r)
