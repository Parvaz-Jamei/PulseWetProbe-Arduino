#include <PulseWetProbe.h>
#include "PwpExamplePins.h"
#include "PwpExampleUtils.h"

PulseWetProbe probe;

void setup() {
  Serial.begin(115200);
  pwpWaitForSerial();

  probe.enableBoardDefaults();
  PwpBoardCapabilities caps = probe.capabilities();

  Serial.println(F("PulseWetProbe board capability report"));
  Serial.print(F("Family: ")); Serial.println(caps.family);
  Serial.print(F("Profile: ")); Serial.println(caps.recommendedProfile);
  Serial.print(F("ADC bits: ")); Serial.println(caps.defaultAdcBits);
  Serial.print(F("Max ADC bits: ")); Serial.println(caps.maxAdcBits);
  Serial.print(F("Burst samples: ")); Serial.println(caps.recommendedBurstSamples);
  Serial.print(F("Tiny memory: ")); Serial.println(caps.memoryTiny ? F("yes") : F("no"));
  Serial.print(F("analogReadResolution: ")); Serial.println(caps.hasAnalogReadResolution ? F("yes") : F("no"));
  Serial.print(F("Native touch: ")); Serial.println(caps.hasNativeTouch ? F("yes") : F("no"));
  Serial.print(F("JSON: ")); Serial.println(caps.supportsJsonOutput ? F("yes") : F("no"));
  Serial.print(F("Fault log: ")); Serial.println(caps.supportsFaultLog ? F("yes") : F("no"));
  Serial.print(F("Advanced filters: ")); Serial.println(caps.supportsAdvancedFilters ? F("yes") : F("no"));
  Serial.print(F("Notes: ")); Serial.println(caps.notes);
}

void loop() {}
