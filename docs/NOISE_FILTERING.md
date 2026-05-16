# Noise Filtering

PulseWetProbe uses fixed-size filters suitable for small MCUs. Filtering is dependency-free and heap-free.

## Filter stages

1. Burst sampling with optional trimmed mean.
2. Optional Hampel spike guard outside Tiny mode.
3. Optional outlier rejector.
4. Optional median filter.
5. Optional moving average.
6. EMA/IIR smoothing.

## Hampel behavior

As of 0.3.5, Hampel filtering uses a dedicated ring buffer. It no longer reads stale or unrelated moving-average data. If a sample deviates from the Hampel median by more than `hampelThreshold`, the median is used and inserted into the Hampel window instead of the spike.

Hampel remains disabled by default on AVR/Tiny mode.

## Noise-aware behavior

`applyFilters(raw, noise)` uses the named `noise` score; the parameter is intentionally part of the filter contract:

- high noise tightens the outlier step limit,
- high noise reduces EMA alpha to smooth more aggressively,
- static presets are still deterministic and fixed-memory.

This is a conservative noise-aware filter, not a certified signal-processing model. Hampel spike rejection is tested by host smoke tests when `PWP_ENABLE_HAMPEL_FILTER` is enabled.

## CSV columns

`trendPermille` is the heuristic conductivity-trend/change score. It is not the same value as `wetPermille` and must not be interpreted as EC or salinity.
