#include <PulseWetProbe.h>
#include "PwpExamplePins.h"
#include "PwpExampleUtils.h"

PulseWetProbe probe;
char csvLine[PWP_CSV_BUFFER_SIZE];
int dryRaw = -1;
int wetRaw = -1;

void printHelp() {
  Serial.println(F("Commands:"));
  Serial.println(F("  d  capture current filtered value as DRY"));
  Serial.println(F("  w  capture current filtered value as WET"));
  Serial.println(F("  c  apply calibration if both points exist"));
  Serial.println(F("  r  print one reading"));
}

void setup() {
  Serial.begin(115200);
  pwpWaitForSerial();

  probe.enableBoardDefaults();
  probe.beginTwoPlate(PWP_EXAMPLE_SENSE_PIN, PWP_EXAMPLE_EXCITE_PIN);
  probe.setProfile(PwpProfile::SOIL);
  probe.filters().setPreset(PwpFilterPreset::INDUSTRIAL_STABLE);

  printHelp();
}

void loop() {
  PwpReading reading = probe.read();

  if (Serial.available()) {
    char cmd = (char)Serial.read();
    if (cmd == 'd') {
      dryRaw = reading.filtered;
      Serial.print(F("Dry raw captured: "));
      Serial.println(dryRaw);
    } else if (cmd == 'w') {
      wetRaw = reading.filtered;
      Serial.print(F("Wet raw captured: "));
      Serial.println(wetRaw);
    } else if (cmd == 'c') {
      if (dryRaw >= 0 && wetRaw >= 0 && dryRaw != wetRaw) {
        probe.calibrateDryWet(dryRaw, wetRaw);
        Serial.println(F("Calibration applied."));
      } else {
        Serial.println(F("Capture valid dry and wet points first."));
      }
    } else if (cmd == 'r') {
      if (reading.toCsv(csvLine, sizeof(csvLine)) > 0) {
        Serial.println(csvLine);
      }
    } else {
      printHelp();
    }
  }

  delay(250);
}
