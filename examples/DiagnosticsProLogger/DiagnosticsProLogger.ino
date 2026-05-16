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
  probe.setProfile(PwpProfile::DIAGNOSTIC);
  probe.calibrateDryWet(120, 850);
  probe.filters().setPreset(PwpFilterPreset::INDUSTRIAL_STABLE);

  pwpPrintCsvHeader();
}

void loop() {
  PwpReading r = probe.read();
  if (r.toCsv(csvLine, sizeof(csvLine)) > 0) {
    Serial.println(csvLine);
  }
  delay(probe.nextIntervalMillis());
}
