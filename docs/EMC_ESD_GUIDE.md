# EMC and ESD Guide

PulseWetProbe is a software library. Industrial reliability depends on the external electrode, cable, enclosure and protection design.

## Minimum protection policy

| Risk | Practical mitigation |
| --- | --- |
| ADC overvoltage | Keep all electrode voltages inside board input limits. Add a series resistor on excitation paths. |
| Continuous DC corrosion | Use high/low pulsed drive and Hi-Z rest. Avoid permanent DC bias across electrodes. |
| Cable pickup | Keep leads short, twist/shield where appropriate, log noise, and validate in the real enclosure. |
| ESD/surge | Add external ESD/surge protection in outdoor or long-cable deployments. |
| Ground loops | Avoid unknown industrial voltages and undocumented ground references. |
| Moisture ingress | Use sealed connectors and materials compatible with the target medium. |

## Software-side support

The library provides:

- burst spread logging,
- cable-noise suspicion flag,
- stuck-reading detection,
- open/short/saturation checks,
- polarization asymmetry / corrosion-risk suspicion,
- quality score reduction when confidence is low.

These flags help detect problems, but they do not replace hardware protection.

## Owner-side evidence required

For public or industrial claims, record:

- board model and core version,
- electrode material and geometry,
- cable length and shield/grounding method,
- resistor values,
- enclosure details,
- dry/wet/brine/soil trend logs,
- known failure-mode tests,
- date and environment.
