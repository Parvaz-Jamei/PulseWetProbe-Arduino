#include <Arduino.h>
#include <PulseWetProbe.h>

PulseWetProbe probe;
char line[PWP_CSV_BUFFER_SIZE];

void setup() {
  Serial.begin(115200);
  probe.enableBoardDefaults();
#if defined(A0)
  const int sensePin = A0;
#else
  const int sensePin = 0;
#endif
  probe.beginSoil(sensePin, 7);
  probe.calibrateDryWet(120, 850);
}

void loop() {
  PwpReading r = probe.read();
  r.toCsv(line, sizeof(line));
  Serial.println(line);
  delay(probe.nextIntervalMillis());
}
