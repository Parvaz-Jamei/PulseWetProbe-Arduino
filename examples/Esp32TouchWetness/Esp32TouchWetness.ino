#include <PulseWetProbe.h>
#include "PwpExamplePins.h"
#include "PwpExampleUtils.h"

PulseWetProbe probe;
char csvLine[PWP_CSV_BUFFER_SIZE];

void setup() {
  Serial.begin(115200);
  pwpWaitForSerial();

  probe.enableBoardDefaults();

#if PWP_ENABLE_ESP32_TOUCH
  if (!probe.beginEsp32TouchWetness(PWP_EXAMPLE_TOUCH_PIN)) {
    Serial.println(F("Touch not available on this board/pin. Falling back to two-plate example pins."));
    probe.beginWetDry(PWP_EXAMPLE_SENSE_PIN, PWP_EXAMPLE_EXCITE_PIN);
  }
#else
  Serial.println(F("ESP32 touch support is not compiled for this target. Falling back to two-plate example pins."));
  probe.beginWetDry(PWP_EXAMPLE_SENSE_PIN, PWP_EXAMPLE_EXCITE_PIN);
#endif

  probe.calibrateDryWet(120, 850);
  pwpPrintCsvHeader();
}

void loop() {
  PwpReading r = probe.read();
  if (r.toCsv(csvLine, sizeof(csvLine)) > 0) {
    Serial.println(csvLine);
  }
  delay(probe.nextIntervalMillis());
}
