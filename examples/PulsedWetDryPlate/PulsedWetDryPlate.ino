#include <PulseWetProbe.h>
#include "PwpExamplePins.h"
#include "PwpExampleUtils.h"

PulseWetProbe probe;
char csvLine[PWP_CSV_BUFFER_SIZE];

void setup() {
  Serial.begin(115200);
  pwpWaitForSerial();

  probe.enableBoardDefaults();
  probe.beginTwoPlate(PWP_EXAMPLE_SENSE_PIN, PWP_EXAMPLE_EXCITE_PIN);
  probe.setProfile(PwpProfile::WETNESS);
  probe.setExcitation(PwpExcitation::PULSED_HIGH_LOW);

  probe.acquisition()
       .setSettlingMicros(100)
       .setDummyReads(2)
       .setBurstSamples(8);

  probe.filters().setPreset(PwpFilterPreset::STABLE);
  probe.calibrateDryWet(80, 760);

  Serial.println(F("PulseWetProbe pulsed wet/dry example"));
  pwpPrintCsvHeader();
}

void loop() {
  PwpReading r = probe.read();
  if (r.toCsv(csvLine, sizeof(csvLine)) > 0) {
    Serial.println(csvLine);
  }
  delay(probe.nextIntervalMillis());
}
