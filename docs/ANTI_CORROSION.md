# Anti-Corrosion Design Notes

PulseWetProbe reduces electrode stress through pulsed drive and Hi-Z rest. It does **not** guarantee corrosion-free operation.

## Current modes

| Mode | What it does | Anti-corrosion value |
| --- | --- | --- |
| `beginTwoPlate()` / `PULSED_HIGH_LOW` | Drives one electrode HIGH/LOW and senses the other electrode. | Low duty cycle and Hi-Z rest reduce continuous DC bias, but this is not true electrode role reversal. |
| `beginReversibleTwoPlate()` | Swaps drive/sense roles between two pins each acquisition cycle. | Better electrode role balancing when both pins and wiring are suitable. |
| ESP32 touch | Uses capacitive touch hardware when supported. | Avoids direct DC conduction model, but still needs board/pin validation. |

## Required hardware practice

- Use a current-limiting resistor in the drive path.
- Add a bleed/reference resistor when the dry sense electrode would float.
- Keep outdoor cables short or shielded where possible.
- Use corrosion-resistant electrode materials for long-term deployments.
- Collect owner-side CSV evidence before claiming improved electrode lifetime.

## Diagnostics

`corrosionRisk` and `foulingScore` are heuristic indicators based on charge-balance debt, polarity imbalance, drift, stuck readings, noisy readings, and possible wiring faults. They are not certified corrosion or fouling measurements.
