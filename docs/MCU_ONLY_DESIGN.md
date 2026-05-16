# MCU-only Design

PulseWetProbe is intentionally MCU-only in Phase 0-2:

- No mandatory external ADC.
- No mandatory capacitance IC.
- No mandatory conductivity chip.
- No mandatory op-amp.
- No mandatory wireless, storage, JSON, or RTOS dependency.

## Why

The goal is an Arduino Library Manager-ready sensing core that works from small AVR boards to ESP32/ARM-class boards. Hardware-specific improvement paths remain optional and guarded.

## Anti-corrosion behavior

The library uses short excitation windows and returns the excitation pin to Hi-Z rest after measurements. High/low pulsed drive is tracked with a charge-balance heuristic. This reduces DC bias exposure but does not eliminate electrochemical corrosion; electrode material, medium chemistry, pulse duty cycle, and current limiting still matter.

## Board-aware behavior

- AVR gets Tiny mode.
- ESP8266 keeps one-ADC limitations explicit.
- ESP32 touch is guarded and optional.
- Full-capable boards can use larger filters, JSON, fault logs, and calibration profiles.

## No false precision

The library reports normalized wetness and conductivity trend indicators. It does not report certified EC, salinity, freezing point, road ice state, or universal soil moisture percentage.
