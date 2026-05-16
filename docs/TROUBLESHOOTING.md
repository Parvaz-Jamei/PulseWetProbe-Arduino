# Troubleshooting

## Readings are random in dry air

The analog sense electrode may be floating. Add a high-value bleed/reference resistor from the sense node to GND/reference and keep leads short.

## Everything reads wet or saturated

Check for shorts, water bridges, too-low series resistance, or calibration values that are too close together.

## `possible_open` or `possible_short` appears

These flags are intentionally soft at first. Hard `open`/`short` faults require repeated windows. Confirm wiring and calibration before treating the result as a real hardware fault.

## ESP32 touch does not start

Use a touch-capable GPIO for the exact ESP32 variant. C3/C6-style targets may not expose native touch. The library returns a `PwpBeginStatus` value from `lastBeginStatus()`.

## JSON is not available

JSON is disabled by default on Tiny/AVR mode. Use `toCsv(char*, size_t)` or enable JSON explicitly only when memory is safe.

## Conductivity trend does not match wetness

That is expected. `conductivityTrend` is a heuristic change/trend score based on the combined level/differential response over time. It is not EC, salinity, or freezing-point measurement.
