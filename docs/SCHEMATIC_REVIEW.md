# Schematic Review Notes

PulseWetProbe does not ship a certified reference PCB. This document gives wiring-level schematic guidance for safe owner-side prototypes.

## Two-plate reference connection

```text
D_EXCITE ---- R_SERIES ---- Plate A
A_SENSE ------------------- Plate B
GND ----------------------- local shield/reference only, not an unknown industrial ground
```

Recommended first-pass `R_SERIES` values:

| Board voltage | Start value | Notes |
| --- | ---: | --- |
| 5 V AVR | 10 kOhm to 47 kOhm | Keep current limited and use short pulses. |
| 3.3 V MCU | 22 kOhm to 100 kOhm | Protect ADC pins and validate settling time. |
| Long cable/outdoor | 47 kOhm to 220 kOhm plus external protection | Slower response; needs hardware validation. |

## Review gates

- ADC pin never exceeds absolute board input range.
- Excitation path has series resistance.
- Electrode fixture cannot short board rails directly.
- Enclosure and connector are suitable for moisture.
- Long cable designs include ESD/surge/EMI protection outside the library.
- Calibration and trend claims are tied to a named electrode geometry.

## Official board schematic references

Do not copy vendor schematics into the repository. Link to official board schematics and verify the exact board revision during validation.
