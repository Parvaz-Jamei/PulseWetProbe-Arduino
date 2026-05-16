#ifndef PWP_EXAMPLE_UTILS_H
#define PWP_EXAMPLE_UTILS_H

#include <Arduino.h>
#include <PulseWetProbe.h>

static void pwpWaitForSerial(unsigned long timeoutMs = PWP_SERIAL_TIMEOUT_MS) {
  const unsigned long start = millis();
  while (!Serial && (millis() - start) < timeoutMs) {
    ;
  }
}

static void pwpPrintCsvHeader() {
  Serial.println(F("seq,ms,rawForward,rawReverse,rawDiff,rawAvg,filtered,touchRaw,burstSpread,wetPermille,levelPermille,diffPermille,trendPermille,quality,noise,stability,drift,fouling,state,flags"));
}

static void pwpPrintBoardHint() {
  Serial.println(F("Hardware note: use a series resistor on the drive path and add a bleed/reference path if the dry sense plate would otherwise float."));
}

#endif
