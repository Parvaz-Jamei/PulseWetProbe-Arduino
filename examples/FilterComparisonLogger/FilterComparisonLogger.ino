#include <PulseWetProbe.h>
#include "PwpExamplePins.h"
#include "PwpExampleUtils.h"

PulseWetProbe responsiveProbe;
PulseWetProbe stableProbe;

void setup() {
  Serial.begin(115200);
  pwpWaitForSerial();

  responsiveProbe.enableBoardDefaults();
  responsiveProbe.beginTwoPlate(PWP_EXAMPLE_SENSE_PIN, PWP_EXAMPLE_EXCITE_PIN);
  responsiveProbe.setProfile(PwpProfile::SOIL);
  responsiveProbe.filters().setPreset(PwpFilterPreset::RESPONSIVE);
  responsiveProbe.calibrateDryWet(120, 850);

  stableProbe.enableBoardDefaults();
  stableProbe.beginTwoPlate(PWP_EXAMPLE_SENSE_PIN, PWP_EXAMPLE_EXCITE_PIN);
  stableProbe.setProfile(PwpProfile::SOIL);
  stableProbe.filters().setPreset(PwpFilterPreset::INDUSTRIAL_STABLE);
  stableProbe.calibrateDryWet(120, 850);

  Serial.println(F("ms,responsive_filtered,responsive_wet_permille,stable_filtered,stable_wet_permille"));
}

void loop() {
  PwpReading a = responsiveProbe.read();
  delay(20);
  PwpReading b = stableProbe.read();

  Serial.print(millis());
  Serial.print(',');
  Serial.print(a.filtered);
  Serial.print(',');
  Serial.print((int)(a.normalizedWetness * 1000.0f));
  Serial.print(',');
  Serial.print(b.filtered);
  Serial.print(',');
  Serial.println((int)(b.normalizedWetness * 1000.0f));

  delay(1000);
}
