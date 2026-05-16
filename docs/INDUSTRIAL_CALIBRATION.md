# Industrial Calibration

PulseWetProbe supports manual dry/wet calibration and optional multi-point calibration when memory allows.

## Simple calibration

```cpp
probe.calibrateDryWet(dryRaw, wetRaw);
```

Measure dry and wet raw values for the actual electrode geometry, board, cable, and medium.

## Multi-point calibration

```cpp
probe.calibration().clearPoints();
probe.calibration().addPoint(120, 0.0f);
probe.calibration().addPoint(420, 0.35f);
probe.calibration().addPoint(850, 1.0f);
```

Multi-point calibration is disabled or reduced on Tiny targets through fixed compile-time limits.

## Profile integrity

Profile export/import uses CRC-16/CCITT-FALSE:

- polynomial: `0x1021`
- initial value: `0xFFFF`
- stable quantized profile fields
- `crc` field excluded from the CRC input

Corrupted profile data must be rejected by `importProfile()`.

## Temperature hook

A simple coefficient hook exists, but temperature-compensated moisture/conductivity accuracy is not validated in this package.
