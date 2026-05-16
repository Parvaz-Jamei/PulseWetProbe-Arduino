# Changelog

## 0.3.7 - Bug Fix

- **Fixed `applyMovingAverage` circular-buffer indexing bug (silent stale output):**
  The sum loop iterated from index `0` to `_movingCount-1` linearly, but the write
  pointer advanced circularly up to `PWP_MOVING_AVG_WINDOW`. When `movingAverageWindow()`
  (default 4) was smaller than `PWP_MOVING_AVG_WINDOW` (8), samples written to indices
  `>= movingAverageWindow()` were never read, causing the moving average to freeze at
  the first window-fill and silently ignore all subsequent samples until the write
  pointer wrapped back to index 0. Fixed by computing the circular start offset and
  reading exactly the most-recent `_movingCount` samples.

- **Fixed `computeConductivityTrend` weight-selection priority:**
  When a `CONDUCTIVITY_TREND` or `SURFACE_BRINE_TREND` profile was combined with
  reversible two-plate mode, the reversible-mode `if` block unconditionally overwrote
  the profile-specific weights, producing incorrect trend composites. The two branches
  are now an `if / else if` chain (profile-specific > reversible > default), ensuring
  `CONDUCTIVITY_TREND` profile weights are honoured regardless of excitation mode.

- Added two host-smoke regression tests covering both fixes (fail codes 26-29).

## 0.3.6 - Pre-public Bug Fix

- Fixed reversed calibration normalization when `dryRaw > wetRaw`: the erroneous `1 - value` flip was removed; the division formula already maps dry→0 and wet→1 in both wiring orientations.
- Fixed `beginTwoPlate()` and `beginReversibleTwoPlate()` to reject same-pin assignments (`INVALID_PIN`) before hardware damage can occur.
- Added compile-time `#error` guards in `PwpConfig.h`: opting in `PWP_ENABLE_HAMPEL_FILTER=1` or `PWP_ENABLE_FAULT_LOG=1` on Tiny/AVR (where window/capacity are 0) now fails at compile time with a clear message instead of producing a zero-size array or modulo-by-zero.
- Fixed `arduino-lint` CI workflow: changed `library-manager: update` to `library-manager: submit` so CI does not fail before the library is in the Arduino Library Manager index.
- Fixed ESP32 reversible-mode example pin defaults: `PWP_EXAMPLE_PLATE_B_PIN`, `PWP_EXAMPLE_REV_A_PIN`, and `PWP_EXAMPLE_REV_B_PIN` no longer default to GPIO34/35 (input-only on classic ESP32); ESP32 reversible defaults are now GPIO32/33 (ADC1-capable, bidirectional).
- Fixed Mbed/RP2040 board-detection ordering in `PwpBoardCapabilities.h`: `ARDUINO_ARCH_MBED_RP2040` is now excluded from the generic `MBED_OR_NRF52` branch so the Nano RP2040 Connect correctly resolves to the RP2040 profile.
- Made all examples self-contained: `PwpExamplePins.h` and `PwpExampleUtils.h` are now copied into each example folder and included with a local path instead of `../`, preventing build failures in Arduino IDE, Web Editor, and Save As workflows.

## 0.3.5 - Pre-public Release Hardening

- Final release-polish pass: moved quality-score penalty constants, charge-balance/stuck thresholds, inter-sample delays, and adaptive-filter constants to `PwpConfig.h`.
- Improved non-Tiny `String` CSV/JSON wrappers to return explicit truncation sentinel strings when bounded formatting fails.
- Added Release Hardening Round for GitHub, Arduino Library Manager, PlatformIO, examples, docs, and CI readiness.
- Added shared example headers: `examples/PwpExamplePins.h` and `examples/PwpExampleUtils.h`.
- Standardized example begin order and serial timeout handling.
- Added `PWP_CSV_BUFFER_SIZE`, `PWP_JSON_BUFFER_SIZE`, and `PWP_SERIAL_TIMEOUT_MS` constants; enlarged CSV/JSON defaults to 384/512 bytes and made buffer output report truncation by returning `0`.
- Fixed the `PULSED_HIGH_LOW` duplicate-condition bug so `PULSED_HIGH_ONLY` no longer performs a LOW reverse sample.
- Moved conductivity weights and state/noise thresholds to named constants, and added `setConductivityTrendWeights()` for advanced tuning.
- Removed the dead `PWP_DIAGNOSTIC_MODE` macro and centralized `analogReadResolution()` through `applyAnalogResolution()`.
- Hardened Hampel filtering with a dedicated sample window instead of stale moving-average state.
- Made `applyFilters()` actually use the `noise` parameter for adaptive outlier/EMA behavior.
- Reworked `conductivityTrend` as a heuristic trend/change score instead of duplicating normalized wetness.
- Reworked `foulingScore` to use drift, polarity imbalance, stuck/noisy readings, possible wiring faults, and corrosion-risk signals instead of instantaneous wetness.
- Replaced the weak calibration checksum with CRC-16/CCITT, polynomial `0x1021`.
- Removed the unused burst sample stack array from `acquireBurstAverage()` for better AVR stack safety.
- Added host smoke checks for CRC corruption rejection, Hampel spike rejection, and conductivity-trend behavior.
- Added GitHub community files, issue templates, pull request template, and separated CI workflows.
- Added `ANTI_CORROSION.md`, `POWER_SAVING.md`, `TROUBLESHOOTING.md`, and `RELEASE_NOTES_DRAFT.md`.

## 0.3.3 - Sensing-logic and beginner-API hardening

- Added beginner wrappers: `beginSoil()`, `beginWetDry()`, `beginConductivityTrend()`, `beginEsp32TouchWetness()`, and `calibrateDryWet()`.
- Added `beginReversibleTwoPlate()` / `beginBalancedTwoPlate()` for true two-electrode role swapping.
- Clarified that `beginTwoPlate()` uses high/low pulsed drive, not true electrode polarity reversal.
- Added separate `levelIndex`, `differentialIndex`, and heuristic `conductivityTrend` outputs.
- Expanded reading states with `DAMP`, `SATURATED`, `UNSTABLE`, and `NEEDS_CALIBRATION`; added `wetnessPercent()` and `stateName()`.
- Softened open/short detection into `possibleOpen` / `possibleShort` first, with repeated-window confirmation for hard faults.
- Fixed first-reading drift false positives by passing previous-filter validity into diagnostics.
- Added floating-input/bleed-resistor wiring guidance and reversible two-plate example.

## 0.3.2 - Pre-public detection/API hardening

- Re-verified megaAVR detection before classic AVR / `__AVR__`; Nano Every class boards resolve to `megaavr` / `PWP_CORE_MODE`.
- Re-verified Renesas UNO R4 detection for both `ARDUINO_ARCH_RENESAS` and `ARDUINO_ARCH_RENESAS_UNO`.
- Made ESP8266 defaults explicit: `PWP_CORE_MODE=1`, `PWP_FULL_MODE=0`.
- Added `PWP_ENABLE_STRING_OUTPUT`; heap-backed `String` helpers are disabled by default in Tiny mode while bounded buffer APIs remain available.
- Confirmed all examples use bounded serial startup waits and buffer-based CSV output.
- Clarified README Phase 3 wording: source/docs/release prep are included; real CI and hardware validation remain owner-side evidence.

## 0.3.1 - Pre-public Board Detection, AVR-Safe Examples and Phase 3 Evidence Tightening

- Fixed megaAVR profile detection so Nano Every / Uno WiFi Rev2 class targets do not fall into classic AVR Tiny mode when `__AVR__` is also defined.
- Fixed Renesas/UNO R4 detection by accepting both `ARDUINO_ARCH_RENESAS` and `ARDUINO_ARCH_RENESAS_UNO`.
- Added host board-detection regression tests for classic AVR, megaAVR, generic Renesas and UNO R4/Renesas macros.
- Added best-effort ESP32 touch GPIO validation and a new `PwpBeginStatus::TOUCH_UNSUPPORTED_PIN` status for known unsupported touch pins.
- Replaced remaining full-reading `toCsv()` example usage with buffer-based `toCsv(char*, size_t)` and bounded all `while (!Serial)` startup waits with a timeout.
- Updated CI workflow to upload Arduino CLI compile logs so AVR memory usage output can become release evidence after CI runs.
- Clarified README and Phase 3 docs: source/release-prep is included, but public release still requires real Arduino CLI/CI logs and owner-side hardware CSV/plot evidence.
- Re-stated that diagnostics are heuristic/trend-based until validated on real hardware.

## 0.3.0 - Phase 3 Release, Validation, Registry and Critical Signal-Path Fix

- Fixed the high/low pulsed signal path: wetness filtering and normalization now use `rawAverage`; `rawDifferential` remains diagnostics-only for polarization/asymmetry/corrosion-risk suspicion.
- Reworked `enableBoardDefaults()` so ESP32-specific defaults use compile-time `#if` instead of a runtime-looking `if (PWP_BOARD_ESP32)`.
- Added host regression coverage proving `filtered == rawAverage` for high/low pulsed two-plate acquisition.
- Normalized example CSV headers and enlarged CSV buffers to match the actual `toCsv()` column set.
- Added Phase 3 documentation: API guide, EMC/ESD guide, schematic review, validation evidence pack, release checklist and non-claim audit.
- Added `CITATION.cff` and PlatformIO smoke metadata while keeping Arduino Library Manager as the primary package target.
- Strengthened CI with Arduino Lint strict Library Manager mode and host smoke test before Arduino example compilation.

## 0.2.0 - Phase 2 Board-Aware Industrial Hardening

- Added board capability matrix fields and board-aware defaults for AVR, megaAVR, ESP32, ESP8266, SAMD, Renesas, Mbed/nRF52, RP2040, STM32, and Teensy.
- Added guarded ESP32 touch mode via `beginTouch()` only when touch support is compiled for the selected target.
- Added analogReadResolution-aware setup for full-capable board families.
- Added advanced filters outside Tiny mode: trimmed burst mean, outlier rejection, and optional Hampel guard.
- Added multi-point calibration, temperature coefficient hook, calibration profile export/import, and CRC validation.
- Added pro diagnostics: stability, drift, fouling heuristic, cable-noise suspicion, stuck-reading detection, touch unsupported flag, and small fault log outside Tiny mode.
- Added optional JSON output outside Tiny mode while keeping CSV universal.
- Added power-aware interval adjustments based on dry/wet state, charge-balance risk, and stuck-reading suspicion.
- Added Phase 2 examples and hardware wiring/schematic documentation.

## 0.1.0 - Phase 0 + Phase 1 Safe Core

- Initial Arduino library skeleton, metadata, docs, examples, CI workflow.
- Implemented MCU-only two-plate high/low pulsed drive, Hi-Z rest, burst sampling, dry/wet calibration, basic filters, CSV output, and basic diagnostics.
