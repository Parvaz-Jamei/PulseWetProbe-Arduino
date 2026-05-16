#include <PulseWetProbe.h>
#include "PwpExamplePins.h"
#include "PwpExampleUtils.h"

PulseWetProbe probe;
char csvLine[PWP_CSV_BUFFER_SIZE];

void setup() {
  Serial.begin(115200);
  pwpWaitForSerial();

  // Reversible two-plate mode: Plate A and Plate B swap drive/sense roles.
  // Use only pins that are safe for both analog sensing and GPIO drive on your board.
  probe.enableBoardDefaults();
  probe.beginReversibleTwoPlate(PWP_EXAMPLE_REV_A_PIN, PWP_EXAMPLE_REV_B_PIN);
  probe.setProfile(PwpProfile::CONDUCTIVITY_TREND);
  probe.calibrateDryWet(120, 850);

#if defined(ARDUINO_ARCH_ESP8266)
  Serial.println(F("Warning: ESP8266 exposes one user ADC channel; this example compiles but is not the recommended hardware topology."));
#endif

  pwpPrintCsvHeader();
}

void loop() {
  PwpReading r = probe.read();
  if (r.toCsv(csvLine, sizeof(csvLine)) > 0) {
    Serial.println(csvLine);
  }
  delay(probe.nextIntervalMillis());
}
