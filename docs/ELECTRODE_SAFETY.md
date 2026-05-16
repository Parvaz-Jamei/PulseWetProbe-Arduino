# Electrode Safety and Anti-corrosion Notes

PulseWetProbe is a software library. It helps drive electrodes conservatively, but the hardware owner is responsible for electrical safety, probe material, insulation, ESD/surge protection, and environmental validation.

## Implemented safety behaviors

- Pulsed excitation instead of continuous DC.
- Backward-compatible high/low pulsed drive on one excitation electrode.
- True reversible two-plate role swapping with `beginReversibleTwoPlate(pinA, pinB)` when the board pins support it.
- Hi-Z rest after every measurement.
- Charge-balance heuristic and corrosion-risk diagnostic flag.
- Current-limiting guidance in `HARDWARE_WIRING_AND_SCHEMATICS.md`.

## Required hardware safeguards

- Add a series resistor in the excitation path.
- Keep ADC/input voltage inside board limits.
- Avoid direct exposure to industrial voltages.
- Use suitable electrode materials for soil/water/brine experiments.
- For outdoor/road installations, add enclosure, sealing, ESD, surge, and cable shielding design.

## Diagnostics are heuristics

`corrosionRisk`, `foulingScore`, `cableNoiseSuspected`, and `stuckReading` are engineering warning indicators. They are not chemical analysis results.
