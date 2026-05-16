# PulseWetProbe Board Support Matrix

This matrix is a compile-time and documentation contract. It prevents the library from behaving like a toy demo by making board capabilities, defaults, optional features, and unsupported claims explicit.

## Source-grounded rules

- AVR-compatible defaults must remain 10-bit friendly because classic Arduino boards such as UNO map ADC readings into the 0..1023 range.
- Boards supporting `analogReadResolution()` may request 12/14/16-bit API paths, but readings remain board/noise/electrode dependent.
- ESP32 native touch is enabled only when the Arduino-ESP32 target exposes touch sensor support and the selected GPIO is touch-capable for that ESP32 variant. C3/C6-class targets may not provide touch pads.
- ESP8266 has one user ADC channel; multi-channel/four-plate examples are not enabled for it.
- UNO R4/Renesas ADC can use higher resolution via `analogReadResolution()`. Both `ARDUINO_ARCH_RENESAS` and `ARDUINO_ARCH_RENESAS_UNO` are treated as Renesas/UNO R4 profiles, but the ESP32 radio module on UNO R4 WiFi is not used for sensing.

## Matrix

| Family | Examples | Default mode | ADC bits | Features on by default | Optional/fallback | Explicitly not claimed |
| --- | --- | --- | ---: | --- | --- | --- |
| AVR 8-bit | UNO R3, Nano, Pro Mini, Mega | `PWP_TINY_MODE` | 10 | Two-plate pulse, pullup wetness, median 3, moving average 4, EMA, CSV | Manual calibration, diagnostic CSV | JSON, Hampel, big fault log, native touch, dynamic allocation |
| megaAVR | Nano Every, Uno WiFi Rev2 | `PWP_CORE_MODE` | 10/12 | Two-plate pulse, small diagnostics, CSV | Light multi-point calibration | Heavy board-specific APIs; must be detected before classic AVR because `__AVR__` may also be present |
| ESP32 family | ESP32, S2, S3, C3, C6 | `PWP_ESP32_MODE` | 12 | Two-plate pulse, power hooks, advanced filters | `beginTouch()` only when compiled/supported and a touch-capable GPIO is used | Universal touch availability, calibrated EC |
| ESP8266 | NodeMCU, Wemos D1 mini | `PWP_CORE_MODE` (`PWP_FULL_MODE=0` by default) | 10 | One-channel ADC trend mode, pullup wetness, CSV | Board-specific voltage divider awareness by user | Four-plate/multi-channel sensing, native touch |
| SAMD / Arduino ARM | MKR, Zero, Nano 33 IoT | `PWP_FULL_MODE` | 12 | Resolution-aware ADC, advanced filters, multi-point calibration, fault log, JSON | Low-power hooks in sketches | Native touch unless board/core provides it |
| Renesas RA / UNO R4 | UNO R4 WiFi/Minima | `PWP_FULL_MODE` | 14 | Higher-resolution ADC path, advanced filters | Board-specific analog front-end validation | Using UNO R4 WiFi ESP32 module for sensing |
| nRF52 / Mbed | Nano 33 BLE, Nano 33 BLE Sense | `PWP_FULL_MODE` | 12 | Advanced filters, calibration, diagnostics | BLE output examples can be added separately | Mandatory BLE dependency |
| RP2040 / RP2350 | Pico, Nano RP2040 Connect | `PWP_FULL_MODE` | 12 | Robust ADC/timer profile, advanced filters | Future PIO timing optional | Native touch |
| STM32 | STM32duino, Nucleo, Blue Pill | `PWP_FULL_MODE` | 12+ | ADC/oversampling-friendly defaults | STM32 low-level calibration only if user adds it | Mandatory STM32 HAL path |
| Teensy | Teensy 3.x/4.x | `PWP_FULL_MODE` | 12+ | High-rate-capable filtering and diagnostics | Teensy-specific ADC/touch extensions later | Teensyduino-only dependency |

## Compile flags

| Flag | Default | Meaning |
| --- | --- | --- |
| `PWP_TINY_MODE` | AVR on, others off | Fixed small buffers; disables heavy features. |
| `PWP_CORE_MODE` | Non-AVR on | Conservative cross-board core features. |
| `PWP_FULL_MODE` | Non-AVR on | Enables full-capable filters/calibration/output paths. |
| `PWP_ENABLE_JSON` | Off on AVR, on elsewhere | Adds `PwpReading::toJson()`. |
| `PWP_ENABLE_FAULT_LOG` | Off on AVR, on elsewhere | Adds small fixed ring buffer for fault snapshots. |
| `PWP_ENABLE_HAMPEL_FILTER` | Off on AVR, on elsewhere | Adds optional Hampel-style outlier guard. |
| `PWP_ENABLE_PROFILE_STORAGE` | Off on AVR, on elsewhere | Adds calibration profile export/import helpers. |
| `PWP_ENABLE_ESP32_TOUCH` | Only compatible ESP32 targets | Allows `beginTouch()`. |

## Fallback policy

1. If native ESP32 touch is not compiled, `beginTouch()` returns `false`; the user must use `beginTwoPlate()`. If the selected touch pin is known to be unsupported, `beginTouch()` returns `false` with `PwpBeginStatus::TOUCH_UNSUPPORTED_PIN`.
2. If board memory is tiny, JSON/Hampel/fault-log features stay off unless the user explicitly opts in.
3. If ADC resolution cannot be set, normalization remains 10-bit-compatible.
4. If a board has only one analog input channel, examples and docs must not imply four-plate support.
5. All outputs remain trend-oriented until owner-side validation proves otherwise for a specific installation.


## Tiny-mode output policy

`PWP_ENABLE_STRING_OUTPUT` defaults to `0` in `PWP_TINY_MODE`. Public examples use bounded buffers to avoid heap fragmentation on small AVR boards.
