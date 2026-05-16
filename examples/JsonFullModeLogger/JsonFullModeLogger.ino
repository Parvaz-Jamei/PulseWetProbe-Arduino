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
}

void loop() {
  PwpReading r = probe.read();
#if PWP_ENABLE_JSON
  char json[PWP_JSON_BUFFER_SIZE];
  if (r.toJson(json, sizeof(json)) > 0) {
    Serial.println(json);
  }
#else
  Serial.println(F("JSON output is disabled for this target/profile. Use CSV instead:"));
  r.toCsv(csvLine, sizeof(csvLine));
  Serial.println(csvLine);
#endif
  delay(probe.nextIntervalMillis());
}
