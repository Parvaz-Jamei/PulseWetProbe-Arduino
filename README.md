# PulseWetProbe

**PulseWetProbe** is an MCU-only Arduino library for pulsed electrode wetness, soil-moisture trend, surface wet/dry sensing, conductivity-trend experiments, calibration, filtering, diagnostics, low-power sampling, and anti-corrosion Hi-Z rest behavior.

> **Pre-public validation status:** Source package, docs, metadata, examples, and CI workflows are prepared for review. Real GitHub Actions logs and owner-side hardware CSV/plot validation are still pending. Do not market this as a validated industrial sensor until those evidence files are attached.

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

A bare analog input can float in dry conditions. Use a safe reference path when needed, for example a high-value bleed resistor from the sense electrode to GND/reference, and use a current-limiting resistor on the driven electrode path. See `docs/HARDWARE_WIRING_AND_SCHEMATICS.md` and `docs/SENSING_MODEL.md` before connecting long cables or outdoor electrodes.

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
| Phase 0 | Implemented | Arduino library skeleton, metadata, docs baseline, examples, GitHub-ready files. |
| Phase 1 | Implemented | Safe MCU-only two-plate core, high/low pulsed drive, Hi-Z rest, fixed-memory filters, dry/wet calibration, CSV output, basic diagnostics. |
| Phase 2 | Implemented | Board matrix, analogReadResolution-aware defaults, guarded ESP32 touch, advanced filters outside Tiny mode, multi-point calibration, CRC-16 profile import/export, diagnostics, optional JSON, power-aware intervals. |
| Phase 3 | Source/docs/release-prep implemented; evidence pending | CI workflows, validation templates, release checklist, non-claim audit, PlatformIO manifest, GitHub community files. Real CI logs and hardware evidence remain owner-side. |

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

Advanced acquisition/filter/calibration APIs remain available in `docs/API_GUIDE.md`. Classification thresholds are conservative defaults (`PWP_THRESHOLD_DRY`, `PWP_THRESHOLD_DAMP`, `PWP_THRESHOLD_WET`) and should be validated for each electrode geometry and medium.

## Sensing modes

`beginTwoPlate(sense, excite)` uses high/low pulsed drive: one electrode is driven HIGH/LOW and one electrode is sensed. This is low-pin-count and backward compatible, but it is **not true electrode role reversal**.

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

CSV output uses bounded buffers. The default constants are `PWP_CSV_BUFFER_SIZE=384` and `PWP_JSON_BUFFER_SIZE=512`; `toCsv(char*, size_t)` / `toJson(char*, size_t)` return `0` if the provided buffer is invalid or too small. Non-Tiny `String` convenience wrappers return `csv_truncated` / `json_truncated` if bounded formatting fails; AVR-facing examples use the buffer API.

```cpp
char line[PWP_CSV_BUFFER_SIZE];
if (r.toCsv(line, sizeof(line)) > 0) {
  Serial.println(line);
}
```

Heap-backed `String` helpers are disabled by default in Tiny mode via `PWP_ENABLE_STRING_OUTPUT`.

## Non-claims

PulseWetProbe is **not**:

- a certified RWIS sensor,
- a laboratory EC meter,
- a salinity meter,
- a freezing-point detector,
- a guaranteed corrosion-free system,
- a universal soil-moisture accuracy solution.

Diagnostics such as `corrosionRisk`, `foulingScore`, `cableNoiseSuspected`, `stuckReading`, `driftSuspected`, `noiseScore`, and `stabilityScore` are heuristic/trend-based indicators until validated on a specific board, electrode geometry, cable, medium, and environment.

## Examples

All examples are dependency-free by default, use shared example pins in `examples/PwpExamplePins.h`, use serial startup timeout helpers from `examples/PwpExampleUtils.h`, and avoid heap-backed CSV output on AVR-facing paths.

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

A public stable release should wait for:

- passing GitHub Actions logs,
- memory/size report review for AVR examples,
- owner-side hardware CSV/plots for dry/wet, soil trend, optional brine-like trend, and ESP32 touch if highlighted,
- final `CHANGELOG.md` and GitHub release notes.

## Citation

Use `CITATION.cff` when citing the project in research notes or publications.

## Contributing and support

See `CONTRIBUTING.md`, `SUPPORT.md`, `SECURITY.md`, and the GitHub issue templates. Keep all claims evidence-based and conservative.
