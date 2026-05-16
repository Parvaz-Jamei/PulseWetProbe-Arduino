# Release Notes Draft - PulseWetProbe 0.3.5 Pre-public Release Hardening

## What changed

- Moved quality-score penalties, charge-balance/stuck thresholds, inter-sample delay defaults, and adaptive-filter constants into `PwpConfig.h` for reviewable release tuning.
- Hardened non-Tiny `String` output wrappers to report CSV/JSON truncation with sentinel strings instead of returning a partially formatted buffer.
- Added Release Hardening Round for GitHub, Arduino Library Manager, and PlatformIO readiness.
- Added GitHub community files, issue templates, pull request template, and separated CI workflows.
- Added shared example pin and utility headers.
- Standardized example begin order and serial timeout behavior.
- Added configurable output/serial buffer constants; CSV/JSON defaults are now 384/512 bytes.
- Hardened filtering, CRC, conductivity trend semantics, and fouling heuristic behavior.
- Fixed the `PULSED_HIGH_LOW`/`PULSED_HIGH_ONLY` control-path bug.
- Added named conductivity/classification constants and `setConductivityTrendWeights()`.

## Fixes

- Hampel filtering now uses a dedicated sample window instead of stale moving-average state.
- `applyFilters()` uses the `noise` parameter for adaptive outlier/EMA behavior.
- `conductivityTrend` is no longer equal to normalized wetness; it is an independent heuristic trend/change score.
- Fouling score no longer comes primarily from instantaneous wetness.
- Calibration profile integrity uses CRC-16/CCITT, polynomial `0x1021`.
- `acquireBurstAverage()` no longer allocates an unused sample stack array.
- `PWP_DIAGNOSTIC_MODE` dead macro was removed.
- `analogReadResolution()` handling is centralized through `applyAnalogResolution()`.

## Validated locally

- Host C++11 compile and smoke test.
- Board detection macro smoke tests.
- CRC corruption rejection.
- Hampel spike rejection smoke.
- Conductivity trend behavior smoke.
- `PULSED_HIGH_ONLY` vs `PULSED_HIGH_LOW` control-path smoke.
- JSON metadata parsing.
- ZIP integrity.

## Not yet validated

- Real GitHub Actions run.
- Real Arduino CLI compile logs from CI.
- Real hardware dry/wet/soil/brine-like trend CSV and plots.
- Real ESP32 touch hardware validation.
- Long-cable EMC/ESD validation.

## Known limitations

- No certified RWIS behavior.
- No exact EC/salinity/freezing-point output.
- No universal soil moisture accuracy.
- Diagnostics remain heuristic/trend-based.
- Four-plate acquisition is still roadmap, not implemented.

## No-claim statement

PulseWetProbe is a trend-oriented Arduino sensing library. It must not be marketed as a certified industrial sensor or precision chemical/road-weather instrument without separate validation evidence.
