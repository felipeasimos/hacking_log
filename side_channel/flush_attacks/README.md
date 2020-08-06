# Flush Attacks

Cache side channel attacks that mainly use time of
execution of some operations to gather information
about other processes in the system.

## Flush+Flush And Flush+Reload Threshold Finder

Given two histograms, one for the misses and one
for the hits, the program determine the best threshold
by:

1. Finding where the second histogram (the one that comes
later in the plotter) starts. We call this point __valley__.

2. Finding the __peak__ of the first.

3. Within the boundaries of the __peak__ and the
__valley__, find the timestamp in which the sum of the
histograms values is the greatest.

This policy has a bias that puts the threshold further
from the peak of the second histogram.
