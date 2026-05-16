# Non-Claim Audit

PulseWetProbe must stay honest and trend-oriented until hardware evidence proves otherwise.

## Forbidden claims before evidence

- Certified RWIS output.
- Exact road ice detection.
- Exact freezing point.
- Exact EC or salinity measurement.
- Universal soil-moisture percentage.
- Universal accuracy across all electrodes, soils, boards and temperatures.
- Medical, safety-critical, automotive or regulatory certification.

## Allowed wording

- Wet/dry trend detection.
- Soil-moisture index after local calibration.
- Conductivity trend experiments.
- Brine-like response logging.
- Electrode polarization/asymmetry diagnostics.
- Board-aware Arduino library.

## README/API claim status

| Area | Status |
| --- | --- |
| Certified RWIS | Not claimed. |
| Exact EC/salinity | Not claimed. |
| Exact freezing point | Not claimed. |
| Universal accuracy | Not claimed. |
| Hardware validation | Marked owner-side. |
| Board support | Compile-time and conservative; real hardware still requires validation. |


## Diagnostics wording

Allowed: heuristic/trend-based corrosion, fouling, cable-noise, drift and stability indicators.
Forbidden: certified fault detection, guaranteed corrosion prediction, universal fouling measurement, or validated cable diagnostics without hardware evidence.
