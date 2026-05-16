# Phase 0 + Phase 1 Implementation Scope

This document remains as historical scope evidence for the original safe core. Phase 2 extends it; see `PHASE_2_IMPLEMENTATION.md`.

## Phase 0 complete

- Arduino library folder structure.
- `library.properties` and `library.json`.
- Public header `PulseWetProbe.h`.
- Main class `PulseWetProbe`.
- Examples and baseline docs.
- CI workflow skeleton.

## Phase 1 complete

- Two-plate MCU-only acquisition.
- High/low pulsed drive with Hi-Z rest.
- Hi-Z rest.
- Settling delay, dummy reads, burst sampling.
- Median, moving average, EMA/IIR, hysteresis.
- Dry/wet calibration.
- Raw/filtered/normalized reading outputs.
- CSV output.
- Basic diagnostics.

## Phase 2 extension

Phase 2 adds board-aware defaults, guarded ESP32 touch, analogReadResolution-aware setup, advanced filters, multi-point calibration, pro diagnostics, fixed fault log, optional JSON, and hardware wiring/schematic documentation.
