# Source Research Log

This package was aligned against the following public documentation sources during release hardening.

## Arduino Library Specification

- `library.properties` must be in the library root.
- The `src/` layout and `includes=PulseWetProbe.h` are kept Library Manager-friendly.
- Examples are kept under `examples/ExampleName/ExampleName.ino`.

Source: https://docs.arduino.cc/arduino-cli/library-specification/

## Arduino Lint

- CI uses `arduino/arduino-lint-action@v2`.
- Strict compliance and Library Manager mode are configured in `.github/workflows/arduino-lint.yml`.

Source: https://github.com/arduino/arduino-lint-action

## Arduino compile-sketches

- CI uses the official `arduino/compile-sketches@v1` action in `.github/workflows/compile-examples.yml`.
- The action compiles sketches and can produce reports for compile/warning/size review.

Source: https://github.com/arduino/compile-sketches

## PlatformIO library.json

- `library.json` is kept as the PlatformIO manifest.
- The manifest includes framework/platform metadata and export exclusions.

Source: https://docs.platformio.org/en/latest/manifests/library-json/index.html

## Arduino Library Registry

- Public submission should wait for a public GitHub repo, release tag, correct `library.properties`, Arduino Lint pass, compile examples pass, and clear validation status.

Source: https://github.com/arduino/library-registry
