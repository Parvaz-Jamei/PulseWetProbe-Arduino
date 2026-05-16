# Power Saving

PulseWetProbe includes a conservative `nextIntervalMillis()` helper. It adjusts the next sampling interval based on state, charge-balance risk, and stuck-reading suspicion.

## What the library does

- Keeps acquisition burst sizes board-aware.
- Returns a suggested interval for the next sample.
- Leaves pins in Hi-Z rest after excitation.
- Avoids mandatory deep-sleep dependencies.

## What the sketch must do

Real low-power modes are board-specific. The user sketch should call the appropriate board sleep API if needed. The library intentionally does not require LowPower, WiFi, BLE, or vendor sleep dependencies.

## Tiny mode

AVR/Tiny mode keeps buffers small and avoids default JSON/Hampel/fault-log features.
