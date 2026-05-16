# PulseWetProbe API Guide

PulseWetProbe exposes a small, fixed-memory Arduino API for pulsed two-plate electrode sensing. The API is designed to remain usable on AVR while allowing richer diagnostics on larger MCUs.

## Beginner setup

Use wrappers when you want a small Arduino-style entry point without tuning every acquisition/filter option first.

```cpp
#include <PulseWetProbe.h>

PulseWetProbe probe;

void setup() {
  Serial.begin(115200);
  probe.beginSoil(A0, 7);
  probe.calibrateDryWet(120, 850);
}

void loop() {
  PwpReading r = probe.read();
  Serial.print(r.wetnessPercent());
  Serial.print(" %, ");
  Serial.println(r.stateName());
  delay(probe.nextIntervalMillis());
}
```

Beginner wrappers:

| API | Purpose |
| --- | --- |
| `beginSoil(sense, excite)` | Soil/moisture trend defaults using safe two-plate high/low pulsed drive. |
| `beginWetDry(sense, excite)` | Faster wet/dry surface response defaults. |
| `beginConductivityTrend(sense, excite)` | Heuristic conductivity-trend logging; not EC/salinity precision. |
| `beginEsp32TouchWetness(pin)` | ESP32 touch-mode wrapper; requires a touch-capable GPIO. |
| `calibrateDryWet(dry, wet)` | Simple wrapper around dry/wet calibration. |
| `PwpReading::wetnessPercent()` | `normalizedWetness * 100`, user-friendly but still calibration-dependent. |
| `PwpReading::stateName()` | Readable state label: dry/damp/wet/saturated/unstable/needs_calibration. |

## Advanced setup

```cpp
#include <PulseWetProbe.h>

PulseWetProbe probe;
char csvLine[PWP_CSV_BUFFER_SIZE];

void setup() {
  Serial.begin(115200);
  probe.enableBoardDefaults();
  probe.beginTwoPlate(A0, 7);
  probe.setProfile(PwpProfile::SOIL);
  probe.setExcitation(PwpExcitation::PULSED_HIGH_LOW);
  probe.calibration().setDryWet(120, 850);
}

void loop() {
  PwpReading r = probe.read();
  r.toCsv(csvLine, sizeof(csvLine));
  Serial.println(csvLine);
  delay(probe.nextIntervalMillis());
}
```

## Core classes and structs

| Type | Purpose |
| --- | --- |
| `PulseWetProbe` | Main object controlling pins, acquisition, filtering, calibration and diagnostics. |
| `PwpReading` | One sampled reading: raw forward/reverse, average, differential, filtered value, wetness, quality and flags. |
| `PwpDiagnostics` | Fault and confidence flags. |
| `PwpBoardCapabilities` | Board-family defaults and compile-time feature availability. |
| `PwpCalibrationConfig` | Dry/wet and multi-point calibration. |
| `PwpCalibrationProfile` | Fixed-size export/import profile with CRC. |

## Acquisition policy

PulseWetProbe now exposes two distinct two-plate models:

| Mode | API | Physical meaning |
| --- | --- | --- |
| High/low pulsed drive | `beginTwoPlate(sense, excite)` + `PULSED_HIGH_LOW` | One electrode is driven HIGH/LOW and the other electrode is sensed. Backward-compatible, low-pin-count, but not true electrode role reversal. |
| Reversible two-plate | `beginReversibleTwoPlate(plateA, plateB)` | Plate B drives while Plate A is sensed, then Plate A drives while Plate B is sensed. This is the stronger anti-bias topology when both pins are suitable for analog/GPIO use. |

The reading separates three concepts instead of hiding them behind one value:

| Field | Meaning |
| --- | --- |
| `levelIndex` / `normalizedWetness` | Main wetness/moisture level from the filtered absolute response. |
| `differentialIndex` | Forward/reverse asymmetry normalized to ADC range; useful for polarization/wiring/asymmetry trends. |
| `conductivityTrend` | Heuristic change/trend score derived from a smoothed level+differential composite over time; not a calibrated EC/salinity measurement. Default weights are named `PWP_CONDUCTIVITY_*` constants and may be overridden with `setConductivityTrendWeights(levelWeight, differentialWeight)`. |

`rawDifferential` is no longer used as the sole moisture signal. It remains visible for diagnostics and trend analysis. `PULSED_HIGH_ONLY` takes only the high-drive sample and mirrors it into `rawReverse`; `PULSED_HIGH_LOW` takes a real low-drive reverse sample.

### Classification thresholds

`DRY`, `DAMP`, `WET`, and `SATURATED` use named defaults from `PwpConfig.h`: `PWP_THRESHOLD_DRY`, `PWP_THRESHOLD_DAMP`, and `PWP_THRESHOLD_WET`. Noise-based `UNSTABLE` classification uses `PWP_NOISE_UNSTABLE_TINY` / `PWP_NOISE_UNSTABLE_CORE`. Treat these as starting points, not universal material thresholds.

## Output

### CSV

`PwpReading::toCsv(char*, size_t)` is the preferred output on AVR because it avoids heap allocation. Use `PWP_CSV_BUFFER_SIZE` (384 bytes by default). The function returns `0` if the buffer is invalid or too small. Non-Tiny `String` wrappers return `csv_truncated` or `json_truncated` when bounded formatting fails; do not rely on heap-backed `String` output in Tiny/AVR builds.

CSV columns:

```text
seq,ms,rawForward,rawReverse,rawDiff,rawAvg,filtered,touchRaw,burstSpread,wetPermille,levelPermille,diffPermille,trendPermille,quality,noise,stability,drift,fouling,state,flags
```

### JSON

`toJson()` and `toJson(char*, size_t)` exist only when `PWP_ENABLE_JSON` is compiled. JSON is disabled by default on AVR Tiny mode. Use `PWP_JSON_BUFFER_SIZE` (512 bytes by default) for bounded output.

`String`-returning helpers (`toCsv()` and `toJson()`) are guarded by `PWP_ENABLE_STRING_OUTPUT`. This is disabled by default in `PWP_TINY_MODE`; use the bounded buffer APIs on AVR and other small-memory deployments.

## Calibration

Use `setDryWet(dryRaw, wetRaw)` for simple two-point calibration. On non-Tiny profiles, add intermediate references with `addPoint(raw, wetness)`.

```cpp
probe.calibration().clearPoints();
probe.calibration().addPoint(120, 0.00f);
probe.calibration().addPoint(420, 0.35f);
probe.calibration().addPoint(850, 1.00f);
```

Calibration points are sorted by raw value. Wetness values are clamped to `0.0..1.0`. Exported profiles use CRC-16/CCITT-FALSE (`0x1021`) and corrupted imports are rejected.

## Board defaults

Call `enableBoardDefaults()` before or after `beginTwoPlate()`. It chooses Tiny/Core/Full/ESP32-safe acquisition and filtering settings from the compile-time board matrix.

## ESP32 touch

`beginTouch(pin)` is guarded. If the target does not expose touch support, it returns `false` and sets `lastBeginStatus()` to `TOUCH_UNSUPPORTED_BOARD`. If the selected GPIO is known to be unsupported for touch, it returns `false` and sets `lastBeginStatus()` to `TOUCH_UNSUPPORTED_PIN`. Use two-plate fallback in examples and applications, and verify the exact ESP32 variant pinout before relying on touch mode.

## Non-claims

The API reports trend-oriented values. It does not claim exact EC, salinity, road-ice state, freezing point, certified RWIS output or universal soil-moisture accuracy.
