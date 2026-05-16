# Security Policy

PulseWetProbe is an Arduino sensing library. It does not process credentials, network traffic, or private data by default.

## Reporting a vulnerability

Please report security-sensitive issues privately to the maintainer email listed in `library.properties` rather than opening a public issue first.

Include:

- PulseWetProbe version
- Board/core version if relevant
- Minimal sketch or reproduction steps
- Why the issue is security-sensitive

## Scope

In scope:

- Unsafe memory behavior in library code
- Build/package issues that could expose unrelated files
- Dangerous example behavior that could damage common boards when followed exactly

Out of scope:

- Hardware misuse outside documented wiring guidance
- Claims of certified wetness/EC/salinity/freezing/RWIS accuracy
