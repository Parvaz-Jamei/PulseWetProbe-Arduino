# Validation Evidence Status

Version: 0.3.5 pre-public release hardening
Date: 2026-05-16

## Source-level checks completed in package generation

| Check | Status |
| --- | --- |
| Host C++11 compile with warnings as errors | PASS |
| Host smoke test | PASS |
| Board-detection macro smoke tests | PASS |
| CRC-16 corrupted-profile rejection smoke | PASS |
| Hampel spike rejection smoke | PASS |
| Conductivity trend behavior smoke | PASS |
| PULSED_HIGH_ONLY vs PULSED_HIGH_LOW control-path smoke | PASS |
| CSV/JSON buffer constants and truncation contract source check | PASS |
| Final-polish source constant check for quality penalties, charge/stuck thresholds, and inter-sample delays | PASS |
| Non-Tiny String wrapper truncation sentinel source check | PASS |
| JSON metadata parse | PASS |
| ZIP integrity | PASS |

## Pending owner-side evidence

| Evidence | Status |
| --- | --- |
| GitHub Actions Arduino Lint strict run | PENDING |
| GitHub Actions compile-sketches matrix | PENDING |
| AVR memory/size report review | PENDING |
| Dry/wet plate CSV + plot | PENDING |
| Soil trend CSV + plot | PENDING |
| Brine-like trend CSV + plot if mentioned in public materials | PENDING |
| ESP32 touch hardware evidence if highlighted | PENDING |
| Long-cable EMC/ESD notes | PENDING |

## Hardware validation requirements

Every hardware evidence entry should include:

- board model,
- Arduino core version,
- exact example/sketch,
- electrode geometry and material,
- series resistor and bleed/reference resistor values,
- cable length,
- dry/wet/calibration procedure,
- CSV sample,
- plot or summary,
- date and operator.

No public stable release should claim validated industrial sensing until the owner-side evidence exists.
