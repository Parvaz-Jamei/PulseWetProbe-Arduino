#include <PulseWetProbe.h>
#include "PwpExamplePins.h"
#include "PwpExampleUtils.h"

PulseWetProbe probe;

void setup() {
  Serial.begin(115200);
  pwpWaitForSerial();

  // Beginner-safe wrapper: board defaults + soil profile + high/low pulsed drive.
  // Hardware note: use a series resistor on the drive path and a bleed/reference
  // path on the sense node when your dry electrode would otherwise float.
  probe.beginSoil(PWP_EXAMPLE_SENSE_PIN, PWP_EXAMPLE_EXCITE_PIN);
  probe.calibrateDryWet(120, 850); // replace with your measured dry/wet raw values

  Serial.println(F("wetnessPercent,state,quality,flags"));
}

void loop() {
  PwpReading r = probe.read();
  char flags[PWP_FLAGS_BUFFER_SIZE];
  r.diagnostics.toFlagString(flags, sizeof(flags));

  Serial.print(r.wetnessPercent(), 1);
  Serial.print(F(","));
  Serial.print(r.stateName());
  Serial.print(F(","));
  Serial.print(r.qualityScore);
  Serial.print(F(","));
  Serial.println(flags);

  delay(probe.nextIntervalMillis());
}
