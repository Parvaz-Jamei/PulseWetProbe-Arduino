# Validation Plan

PulseWetProbe validation is split into source-level checks and owner-side hardware evidence.

## Source-level validation

- Host C++11 compile and smoke test.
- Board macro detection smoke tests.
- CRC-16 profile corruption rejection.
- Hampel spike rejection smoke.
- Conductivity-trend behavior smoke.
- Arduino Lint strict Library Manager mode.
- Arduino example compile matrix.

## Hardware validation

Owner-side validation should be performed before public stable release:

1. Dry/wet electrode plate test.
2. Soil moisture trend test.
3. Conductivity/brine-like trend test only if public docs mention brine-like behavior.
4. ESP32 touch test only if highlighted.
5. Long-cable/EMC notes for industrial environments.

## Evidence format

Use CSV plus a short markdown note containing board, core version, sketch, wiring, resistor values, cable length, electrode material, calibration, and date.
