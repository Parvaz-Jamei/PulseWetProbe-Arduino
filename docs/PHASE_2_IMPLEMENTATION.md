# Phase 2 Implementation Notes

Phase 2 turns the Phase 1 safe core into a board-aware industrial library layer without adding mandatory external hardware or dependencies.

## Implemented

| Requirement | Implementation |
| --- | --- |
| BoardCapabilities | `PwpBoardCapabilities` now exposes memory, ADC resolution, touch, fault log, JSON, profile storage, single-ADC, and four-plate recommendation fields. |
| Board defaults | `enableBoardDefaults()` selects Tiny/Core/Full/ESP32-safe acquisition and filter defaults. |
| ESP32 touch | `beginTouch(pin)` is guarded by `PWP_ENABLE_ESP32_TOUCH`, target touch capability and best-effort GPIO validation. Unsupported targets or known unsupported pins return `false` instead of silently logging invalid readings. |
| ADC resolution | `applyAnalogResolution()` centralizes guarded `analogReadResolution(_adcBits)` calls for known board families that support the API path. |
| Advanced filters | Trimmed burst mean, outlier rejector, optional Hampel guard, median, moving average, and EMA are available without heap allocation. |
| Calibration Pro | Dry/wet calibration plus fixed-size multi-point calibration; temperature compensation hook; CRC profile export/import. |
| Diagnostics Pro | Adds noise, stability, drift, fouling heuristic, cable noise, stuck reading, saturation, corrosion, open/short, touch unsupported. |
| Power | `nextIntervalMillis()` adapts to dry/wet state, charge-balance risk, and stuck-reading suspicion. |
| Outputs | CSV everywhere; JSON only when `PWP_ENABLE_JSON` is compiled. |
| Fault log | Small fixed ring buffer outside Tiny mode; off on AVR by default. |

## Review of Phase 1 issues fixed

- ESP32 touch was only a placeholder. It is now a guarded API with fallback behavior and best-effort touch-capable GPIO validation.
- Board capability fields were too shallow. They now express feature support and limitations.
- Diagnostics were basic. Phase 2 adds pro heuristic indicators while keeping non-claim language.
- Calibration was dry/wet only. Phase 2 adds fixed-size multi-point interpolation and CRC export/import.
- JSON/fault log/Hampel policy was not fully implemented. Defaults now keep them off on Tiny mode and available outside Tiny mode.
- Docs did not include wiring/schematic guidance. Phase 2 adds a dedicated hardware wiring/schematic document.

## Still intentionally deferred

- True four-plate acquisition API is not implemented in Phase 2; docs only prepare the safety/board policy.
- No EEPROM/Flash persistence backend is included; profile export/import is a dependency-free data structure.
- No exact EC, salinity, brine concentration, ice-risk, or freezing-point output is claimed.
- No hardware evidence is bundled because owner-side hardware validation is required.

## Phase 2 signal-path correction in 0.3.3

High/low pulsed acquisition stores both `rawAverage` and `rawDifferential`. The corrected Phase 2 policy is:

- `rawAverage` is the measurement signal used for filtering and wetness normalization.
- `rawDifferential` is diagnostics-only and indicates forward/reverse asymmetry, polarization, wiring imbalance or corrosion-risk suspicion.

This avoids turning polarity asymmetry into a false moisture reading.


## v0.3.3 correction notes

- megaAVR is now detected before classic AVR so boards that also expose `__AVR__` do not fall into `PWP_TINY_MODE`.
- Renesas/UNO R4 detection now accepts both `ARDUINO_ARCH_RENESAS` and `ARDUINO_ARCH_RENESAS_UNO`.
- ESP32 touch now rejects known unsupported pins when the target map can be identified; unknown ESP32-family maps remain documented as requiring a touch-capable GPIO.
- Diagnostics remain heuristic/trend-based until owner-side CSV and hardware evidence exist.

## v0.3.3 sensing-model hardening

- `PwpExcitation::PULSED_HIGH_LOW` is the clear name for the backward-compatible one-drive-pin high/low pulse model.
- `PwpExcitation::PULSED_ALTERNATING` remains only as a backward-compatible alias.
- `beginReversibleTwoPlate(pinA, pinB)` / `beginBalancedTwoPlate(pinA, pinB)` add true two-electrode role swapping for stronger electrode bias balancing.
- `PwpReading` now separates `levelIndex`, `differentialIndex`, and heuristic `conductivityTrend`.
- `possibleOpen` / `possibleShort` are reported before hard `openCircuit` / `shortCircuit`, which require repeated confirmation windows.
- First reading no longer reports drift because diagnostics receive explicit previous-filter validity.
- `DAMP`, `SATURATED`, `UNSTABLE`, and `NEEDS_CALIBRATION` states were added for better user-facing output.

