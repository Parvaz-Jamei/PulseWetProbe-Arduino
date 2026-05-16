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
  probe.setProfile(PwpProfile::SOIL);
  probe.calibrateDryWet(120, 850);
  probe.setBaseIntervalMillis(1000);
}

void loop() {
  PwpReading r = probe.read();
  if (r.toCsv(csvLine, sizeof(csvLine)) > 0) {
    Serial.println(csvLine);
  }

  // The library returns a conservative adaptive interval. Real deep sleep remains sketch/board-specific.
  delay(probe.nextIntervalMillis());
}
