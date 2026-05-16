# Hardware Wiring and Schematic Guidance

PulseWetProbe is an MCU-only electrode sensing library. The library does not require an external ADC, op-amp, conductivity IC, or dedicated capacitive sensor IC. Industrial use still requires careful wiring, current limiting, enclosure design, and owner-side validation.

## Minimal two-plate topology

```text
MCU digital excitation pin ----[ 4.7 kΩ to 100 kΩ series resistor ]---- Plate A
MCU analog sense pin ----------------------------------------------- Plate B
MCU GND ------------------------------------------------------------ shield/reference area
```

Recommended starting values:

| Board class | Series resistor starting point | Notes |
| --- | ---: | --- |
| 5 V AVR UNO/Nano | 10 kΩ to 47 kΩ | Keep current limited; avoid continuous DC by using high/low pulsed mode. |
| 3.3 V ESP32/RP2040/SAMD/nRF52 | 22 kΩ to 100 kΩ | Protect ADC pins; watch ADC input range and board-specific attenuation. |
| Long cable / industrial enclosure | 47 kΩ to 220 kΩ plus shielding strategy | More protection, slower settling; validate noise and response. |



## Reversible two-plate topology

```text
MCU analog/GPIO pin A ----[ optional series/protection resistor ]---- Plate A
MCU analog/GPIO pin B ----[ optional series/protection resistor ]---- Plate B
Optional high-value bleed/reference path where required by the board/probe geometry
```

Use `beginReversibleTwoPlate(pinA, pinB)` only when both pins can be safely used for analog sampling and GPIO drive on the selected board. This mode swaps electrode roles during acquisition and is the preferred software topology for stronger electrode bias balancing.

## Floating input warning

A bare analog input connected only to a dry electrode can float. For beginner-safe and repeatable readings, use one of these reference strategies:

| Strategy | When to use | Notes |
| --- | --- | --- |
| Bleed resistor | Minimal two-plate analog sensing | Add a high-value resistor such as 100 kΩ to 1 MΩ from the sense plate/node to GND/reference so dry conditions are defined instead of random. Validate value with your electrode/cable capacitance. |
| Pullup wetness mode | Coarse wet/dry detection | Uses MCU `INPUT_PULLUP`; good for threshold-like tests, not precision. |
| Reversible two-plate mode | Better anti-bias topology | Both plates connect to pins that can act as GPIO drive and analog sense on the chosen board. |
| ESP32 touch mode | Capacitive pad/probe experiments | Requires a touch-capable GPIO and target support. |

## Pullup wetness topology

```text
MCU pin configured INPUT_PULLUP ---- Plate A
MCU analog/digital sense path ------- Plate B / reference
```

Use this only for coarse wet/dry trend detection. Internal pullups vary by MCU family and are not precision resistors.

## ESP32 touch topology

```text
ESP32 touch-capable GPIO ---- electrode pad / probe plate
GND or shield --------------- local reference plane / guard strategy
```

`beginTouch()` is only available when the selected ESP32 target and Arduino core expose touch support. Some ESP32 families do not have touch pads.

## ADC input protection cautions

- Keep electrode voltage inside the board's valid ADC/input range.
- Never connect electrodes directly to unknown industrial voltages.
- Use a series resistor on excitation paths.
- Use Hi-Z rest between pulses.
- Long outdoor leads need ESD/surge/EMI design outside the library scope.
- For road/ice/brine experiments, treat results as trend evidence, not certified road-weather output.

## Board source references reviewed

The implementation and matrix were aligned with official board/API documentation where possible:

| Topic | Official/technical source used | Design consequence |
| --- | --- | --- |
| Arduino library metadata | Arduino Library Specification | `library.properties` remains in the repository root and `architectures=*` is used with compile-time guards. |
| ADC baseline | Arduino `analogRead()` docs and UNO/Nano schematics | 10-bit-compatible fallback and AVR Tiny mode. |
| ADC resolution | Arduino `analogReadResolution()` docs | Full-capable profiles call `analogReadResolution()` only on families expected to expose it. |
| UNO R4 ADC | Arduino UNO R4 ADC resolution tutorial | Renesas profile uses 14-bit default but no precision claim. |
| ESP32 touch | Arduino-ESP32 Touch API | `beginTouch()` is guarded and non-universal. |
| ESP32 DevKitC | Espressif ESP32-DevKitC docs/schematics resources | ESP32 docs avoid assuming every board exposes every touch/ADC behavior. |
| ESP8266 ADC | ESP8266 Arduino core reference | One-channel ADC limitation is explicit. |
| RP2040 ADC | Arduino-Pico analog docs | RP2040 profile uses 12-bit ADC-oriented defaults. |
| Arduino board schematics | Arduino UNO R3 and Nano official schematic PDFs | Wiring docs avoid relying on hidden hardware assumptions. |

## Owner-side schematic review checklist

Before hardware validation, confirm:

- Exact board model and operating voltage.
- ADC pin maximum input voltage.
- Excitation pin and sense pin are not shorted by the probe fixture.
- Series resistor is installed in the excitation path.
- Electrode material is suitable for the target medium.
- Cable length, shielding, grounding, and enclosure ingress protection are documented.
- Dry/wet calibration procedure is repeatable and logged.
- Brine/road/soil labels are marked as trend tests, not certified measurements.
