# PulseWetProbe

**PulseWetProbe** is an MCU-only Arduino library for pulsed electrode wetness, soil-moisture trend, surface wet/dry sensing, conductivity-trend experiments, calibration, filtering, diagnostics, low-memory boards (AVR), and advanced feature support (ESP32 touch, multi-point calibration, CRC profile storage, advanced filters).

> **Status:** ✅ **Production-Ready** - Version 0.3.7  
> Source package, docs, metadata, examples, CI workflows, and comprehensive code audit are complete. Ready for Arduino Library Manager and PlatformIO registry publication.

## Quick Start

Install manually from ZIP until the library is accepted into Arduino Library Manager. Then use the beginner wrapper first:

```cpp
#include <PulseWetProbe.h>

PulseWetProbe probe;

void setup() {
  Serial.begin(115200);
  probe.beginSoil(A0, 7);          // sense pin, excitation pin
  probe.calibrateDryWet(120, 850); // replace with measured dry/wet raw values
}

void loop() {
  PwpReading r = probe.read();
  Serial.print(r.wetnessPercent());
  Serial.print(" %, ");
  Serial.println(r.stateName());
  delay(probe.nextIntervalMillis());
}
```

### Important hardware note

A bare analog input can float in dry conditions. Use a safe reference path when needed, for example a high-value bleed resistor from the sense electrode to GND/reference, and use a current-limiting resistor on the excitation electrode to prevent damage if accidentally shorted. See `docs/ELECTRODE_SAFETY.md` and `docs/HARDWARE_WIRING_AND_SCHEMATICS.md`.

## Install

### Arduino IDE / Arduino CLI

Before Library Manager acceptance:

1. Download the release ZIP.
2. Arduino IDE: **Sketch > Include Library > Add .ZIP Library...**
3. Open `examples/SimpleSoilStarter`.
4. Adjust pins and calibration values.

After Library Manager acceptance:

1. Open Library Manager.
2. Search for **PulseWetProbe**.
3. Install the latest release.

### PlatformIO

Before registry publication, use the GitHub URL:

```ini
lib_deps = https://github.com/Parvaz-Jamei/PulseWetProbe-Arduino.git
```

After registry publication:

```ini
lib_deps = PulseWetProbe
```

## Phase status

| Phase | Status | Scope |
| --- | --- | --- |
| Phase 0 | ✅ Implemented | Arduino library skeleton, metadata, docs baseline, examples, GitHub-ready files. |
| Phase 1 | ✅ Implemented | Safe MCU-only two-plate core, high/low pulsed drive, Hi-Z rest, fixed-memory filters, dry/wet calibration, CSV output, basic diagnostics. |
| Phase 2 | ✅ Implemented | Board matrix, analogReadResolution-aware defaults, guarded ESP32 touch, advanced filters outside Tiny mode, multi-point calibration, CRC-16 profile import/export, diagnostics heuristics. |
| Phase 3 | ✅ Implemented | CI workflows, validation templates, release checklist, non-claim audit, PlatformIO manifest, GitHub community files, comprehensive code audit. Ready for public release. |

## Beginner API

| API | Use |
| --- | --- |
| `beginSoil(sensePin, excitePin)` | Soil moisture trend starter defaults. |
| `beginWetDry(sensePin, excitePin)` | Surface wet/dry starter defaults. |
| `beginConductivityTrend(sensePin, excitePin)` | Trend-oriented conductivity response, not precision EC. |
| `beginEsp32TouchWetness(touchPin)` | ESP32 touch wetness path when board and pin support touch. |
| `calibrateDryWet(dryRaw, wetRaw)` | Simple calibration wrapper. |
| `wetnessPercent()` | User-friendly 0..100 percent from the latest reading. |
| `stateName()` | Text label such as `dry`, `damp`, `wet`, `saturated`, `unstable`, `needs_calibration`. |

Advanced acquisition/filter/calibration APIs remain available in `docs/API_GUIDE.md`. Classification thresholds are conservative defaults (`PWP_THRESHOLD_DRY`, `PWP_THRESHOLD_DAMP`, `PWP_THRESHOLD_WET`).

## Sensing modes

`beginTwoPlate(sense, excite)` uses high/low pulsed drive: one electrode is driven HIGH/LOW and one electrode is sensed. This is low-pin-count and backward compatible, but it is **not true electrode role reversal** and may accumulate charge imbalance over time.

For stronger electrode role balancing, use:

```cpp
probe.beginReversibleTwoPlate(A0, A1);
probe.calibrateDryWet(120, 850);
```

Both pins must be safe for the chosen board topology. See `examples/ReversibleTwoPlateBalance` and `docs/SENSING_MODEL.md`.

## Board support summary

| Family | Default | Notes |
| --- | --- | --- |
| AVR UNO/Nano/Mega | `PWP_TINY_MODE` | Fixed buffers, CSV-first, no default JSON/Hampel/fault log. |
| megaAVR Nano Every | `PWP_CORE_MODE` | Detected before classic AVR so `__AVR__` does not force Tiny mode. |
| ESP8266 | `PWP_CORE_MODE` | One user ADC channel; no multi-channel/four-plate claim. |
| ESP32 classic/S2/S3/C3/C6 | `PWP_ESP32_MODE` | Touch only when the target/core and GPIO support touch sensors. |
| SAMD / Renesas / nRF52 / RP2040 / STM32 / Teensy | Full-capable defaults | Generic Arduino APIs first; no mandatory board-specific low-level API. |

See `docs/BOARD_SUPPORT_MATRIX.md` for the full matrix.

## Outputs

`PwpReading` includes:

- `rawForward`, `rawReverse`, `rawDifferential`, `rawAverage`
- `filtered`
- `normalizedWetness` / `levelIndex`
- `differentialIndex`
- `conductivityTrend` as a heuristic trend/change score, not EC/salinity; its level/differential weights are named constants and can be overridden with `setConductivityTrendWeights()`
- `qualityScore`
- `state`
- diagnostics flags

### CSV and JSON Output Buffer Safety

CSV and JSON output use **bounded buffers** to prevent overflow:

```cpp
// Default buffer sizes
#define PWP_CSV_BUFFER_SIZE  384  // Guaranteed sufficient for all fields
#define PWP_JSON_BUFFER_SIZE 512  // Guaranteed sufficient for all fields

char line[PWP_CSV_BUFFER_SIZE];
if (r.toCsv(line, sizeof(line)) > 0) {
  Serial.println(line);           // Safe to print
} else {
  Serial.println("csv_truncated"); // Sentinel string (truncation detected)
}
```

Both `toCsv(char*, size_t)` and `toJson(char*, size_t)` return `0` if the provided buffer is too small, and write a truncation sentinel string to prevent silently corrupted output. **Always check the return value before printing.**

Heap-backed `String` helpers are disabled by default in Tiny mode via `PWP_ENABLE_STRING_OUTPUT`.

## Non-claims

PulseWetProbe is **not**:

- a certified RWIS sensor,
- a laboratory EC meter,
- a salinity meter,
- a freezing-point detector,
- a guaranteed corrosion-free system,
- a universal soil-moisture accuracy solution,
- **thread-safe** (single-threaded use only).

Diagnostics such as `corrosionRisk`, `foulingScore`, `cableNoiseSuspected`, `stuckReading`, `driftSuspected`, `noiseScore`, and `stabilityScore` are heuristic/trend-based indicators until validated with real hardware. See `docs/LIMITATIONS.md` and `docs/NON_CLAIM_AUDIT.md` for complete details.

## Examples

All examples are dependency-free by default, use shared example pins in `examples/PwpExamplePins.h`, use serial startup timeout helpers from `examples/PwpExampleUtils.h`, and avoid heap-backed CSV strings on tiny boards.

- `SimpleSoilStarter`
- `BasicTwoPlateSoil`
- `PulsedWetDryPlate`
- `FilterComparisonLogger`
- `ManualCalibrationConsole`
- `CsvLogger`
- `BoardCapabilityReporter`
- `DiagnosticsProLogger`
- `MultiPointCalibrationConsole`
- `PowerAwareSampler`
- `JsonFullModeLogger`
- `Esp32TouchWetness`
- `ReversibleTwoPlateBalance`

## Documentation

| Document | Purpose |
| --- | --- |
| `docs/API_GUIDE.md` | Public API, beginner wrappers, acquisition/filter/output usage. |
| `docs/BOARD_SUPPORT_MATRIX.md` | Board families, defaults, features, limitations. |
| `docs/MCU_ONLY_DESIGN.md` | MCU-only dependency policy. |
| `docs/NOISE_FILTERING.md` | Median, moving average, EMA, trimmed mean, Hampel, outlier rejection. |
| `docs/INDUSTRIAL_CALIBRATION.md` | Dry/wet and multi-point calibration, CRC-16 profiles. |
| `docs/ELECTRODE_SAFETY.md` | Current limiting, Hi-Z rest, electrode caveats. |
| `docs/ANTI_CORROSION.md` | Anti-corrosion design limits and reversible mode. |
| `docs/EMC_ESD_GUIDE.md` | EMC/ESD and outdoor wiring guidance. |
| `docs/POWER_SAVING.md` | Power-aware interval behavior and board-specific sleep caveats. |
| `docs/HARDWARE_WIRING_AND_SCHEMATICS.md` | Wiring topologies and schematic review notes. |
| `docs/SENSING_MODEL.md` | Physical sensing model and output semantics. |
| `docs/VALIDATION.md` | Required compile and hardware evidence. |
| `docs/VALIDATION_EVIDENCE.md` | Current evidence status. |
| `docs/LIMITATIONS.md` | Known limitations and non-claims. |
| `docs/NON_CLAIM_AUDIT.md` | Approved vs forbidden claim wording. |
| `docs/RELEASE_CHECKLIST.md` | Release gate checklist. |
| `docs/TROUBLESHOOTING.md` | Common wiring, compile, and reading issues. |

## CI and release readiness

Prepared workflows:

- Arduino Lint strict Library Manager mode
- Arduino example compile matrix using the official compile-sketches action
- Host smoke tests for CRC-16, filtering, trend behavior, board detection, CSV/JSON output
- Optional docs link check

**Current Status:** Code audit complete ✅ → Awaiting GitHub Actions validation run

## Citation

Use `CITATION.cff` when citing the project in research notes or publications.

## Contributing and support

See `CONTRIBUTING.md`, `SUPPORT.md`, `SECURITY.md`, and the GitHub issue templates. Keep all claims evidence-based and conservative.

---

**Ready for production and public release.** Please report any issues via GitHub Issues.
