# Release Checklist

## Code

- [ ] No unused parameters or dead arrays.
- [ ] No misleading field names.
- [ ] No heavy dynamic allocation in AVR/Tiny mode.
- [ ] `String` helpers are guarded by `PWP_ENABLE_STRING_OUTPUT`.
- [ ] `toCsv(char*, size_t)` is the default example output path.
- [ ] CRC-16 profile import rejects corrupted data.
- [ ] Filtering behavior is documented and smoke-tested.
- [ ] Conductivity trend is not presented as EC/salinity/freezing-point.

## Examples

- [ ] Every example lives in `examples/ExampleName/ExampleName.ino`.
- [ ] Every example is dependency-free by default.
- [ ] Shared pins come from `examples/PwpExamplePins.h`.
- [ ] Shared helpers come from `examples/PwpExampleUtils.h`.
- [ ] Serial wait has a timeout.
- [ ] `PULSED_HIGH_ONLY` and `PULSED_HIGH_LOW` use different acquisition paths.
- [ ] AVR-facing CSV output uses bounded buffers sized by `PWP_CSV_BUFFER_SIZE`.

## Metadata

- [ ] `library.properties` exists in repository root.
- [ ] `src/PulseWetProbe.h` is the public header.
- [ ] `library.properties`, `library.json`, `CITATION.cff`, README, CHANGELOG, and release notes versions match.
- [ ] Git tag matches `library.properties` version, for example `v0.3.5`.

## CI

- [ ] Arduino Lint strict Library Manager mode passes.
- [ ] `arduino/compile-sketches` matrix passes.
- [ ] Host smoke tests pass.
- [ ] AVR compile/memory output is reviewed.

## Hardware evidence

- [ ] Dry/wet plate CSV and plot.
- [ ] Soil trend CSV and plot.
- [ ] Brine-like trend CSV and plot only if public docs highlight it.
- [ ] ESP32 touch hardware evidence if highlighted.
- [ ] Wiring and resistor values recorded.

## Claim control

- [ ] No certified RWIS claim.
- [ ] No exact EC/salinity claim.
- [ ] No freezing-point detection claim.
- [ ] No guaranteed corrosion-free behavior claim.
- [ ] Diagnostics described as heuristic/trend-based.
