# PulseWetProbe Sensing Model

PulseWetProbe separates software acquisition from physical claims. The library provides MCU-only excitation/sampling patterns and heuristic trend outputs; it does not turn bare electrodes into a calibrated EC, salinity, freezing-point, or certified RWIS instrument.

## Two supported two-plate models

| Model | API | Meaning |
| --- | --- | --- |
| High/low pulsed drive | `beginTwoPlate(sense, excite)` + `PwpExcitation::PULSED_HIGH_LOW` | One electrode is driven HIGH/LOW while the other is sensed. This is the backward-compatible low-pin-count mode. It reduces continuous DC exposure via pulsing and Hi-Z rest, but it is not true electrode role reversal. |
| Reversible two-plate role swapping | `beginReversibleTwoPlate(plateA, plateB)` | The library samples Plate A while Plate B drives, then samples Plate B while Plate A drives. This is the preferred software topology for stronger electrode role balancing when both pins support analog sensing and GPIO drive. |

`PwpExcitation::PULSED_ALTERNATING` remains as a backward-compatible alias for `PULSED_HIGH_LOW`, but new examples use the clearer name. `PULSED_HIGH_ONLY` is intentionally different: it performs only the HIGH sample and mirrors that value into `rawReverse` so no LOW reverse sample is taken.

## Output channels

| Field | Source | Use |
| --- | --- | --- |
| `levelIndex` | Filtered absolute response, normalized by calibration | Main wetness/moisture trend. |
| `differentialIndex` | `abs(rawForward - rawReverse)` normalized to ADC range | Polarity/asymmetry/polarization/wiring trend. |
| `conductivityTrend` | Change score from a smoothed level+differential composite over time | Trend logging only; not calibrated EC or salinity. |

## Floating input control

A dry electrode connected only to an analog input can float. Use a bleed/reference path, pullup wetness mode, reversible topology, or ESP32 touch topology depending on the experiment. See `HARDWARE_WIRING_AND_SCHEMATICS.md`.

## Diagnostics policy

Open/short detection is intentionally conservative. The library first reports `possibleOpen` / `possibleShort`; hard `openCircuit` / `shortCircuit` requires repeated windows. This avoids classifying a valid dry/wet endpoint as a hard fault after one sample.
