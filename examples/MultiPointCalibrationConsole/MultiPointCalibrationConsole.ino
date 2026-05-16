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

  probe.calibration().clearPoints();
  probe.calibration().addPoint(120, 0.00f);  // dry reference
#if PWP_MAX_CAL_POINTS > 2
  probe.calibration().addPoint(420, 0.35f);  // damp reference
#endif
  probe.calibration().addPoint(850, 1.00f);  // wet reference

  PwpCalibrationProfile profile = probe.calibration().exportProfile();
  Serial.print(F("Calibration points: "));
  Serial.println(probe.calibration().pointCount());
  Serial.print(F("Profile CRC-16/CCITT: 0x"));
  Serial.println(profile.crc, HEX);
  pwpPrintCsvHeader();
}

void loop() {
  PwpReading r = probe.read();
  if (r.toCsv(csvLine, sizeof(csvLine)) > 0) {
    Serial.println(csvLine);
  }
  delay(probe.nextIntervalMillis());
}
