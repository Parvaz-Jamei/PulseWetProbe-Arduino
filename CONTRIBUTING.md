# Contributing to PulseWetProbe

Thanks for considering a contribution.

## Ground rules

- Keep the library dependency-free by default.
- Do not add certified RWIS, exact EC, salinity, freezing-point, or universal soil-accuracy claims.
- Preserve AVR/Tiny memory safety: fixed buffers, no mandatory heap-heavy output.
- Keep examples beginner-friendly and dependency-free by default.
- Update docs and `CHANGELOG.md` when public behavior changes.

## Development checks

Recommended local checks before opening a pull request:

```bash
g++ -std=c++11 -Wall -Wextra -Werror -pedantic -I src src/PulseWetProbe.cpp test/host/host_smoke.cpp -o /tmp/pwp_host_smoke
/tmp/pwp_host_smoke
```

CI is expected to run Arduino Lint and example compile checks.

## Hardware evidence

Hardware validation is welcome but must include board model, Arduino core version, wiring topology, calibration method, CSV samples, and any plots/notes. Mark unvalidated behavior honestly.
