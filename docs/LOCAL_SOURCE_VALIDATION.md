# Local Source Validation

Version: 0.3.7 bug-fix release
Date: 2026-05-16

## Commands run during package preparation

```bash
g++ -std=c++11 -Wall -Wextra -Werror -pedantic -I src src/PulseWetProbe.cpp test/host/host_smoke.cpp -o /tmp/pwp_host
/tmp/pwp_host
```

Board macro simulations were compiled for:

- classic AVR
- megaAVR + `__AVR__`
- Renesas UNO R4 macro
- generic Renesas macro
- ESP8266
- ESP32
- SAMD
- Mbed/nRF52
- RP2040
- STM32
- Teensy

## Host smoke coverage

- Basic class construction.
- CSV output.
- JSON output when enabled.
- Board capabilities.
- First-reading drift false-positive guard.
- High/low two-plate raw-average signal path.
- Reversible two-plate sample path.
- Beginner API wrapper path.
- CRC-16 profile export/import and corrupted-profile rejection.
- Hampel spike rejection when compiled.
- Conductivity-trend behavior not equal to wetness.

## Not run locally

- Real Arduino CLI compile matrix.
- Arduino Lint binary/action.
- PlatformIO `pio pkg pack`.
- Hardware tests.

These are prepared in workflows/docs but remain owner-side or CI-side validation gates.
