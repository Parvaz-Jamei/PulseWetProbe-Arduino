#include <PulseWetProbe.h>
#include <cstring>

int main(int argc, char** argv) {
  if (argc < 2) return 100;
  PulseWetProbe probe;
  probe.enableBoardDefaults();
  PwpBoardCapabilities caps = probe.capabilities();
  if (std::strcmp(caps.family, argv[1]) != 0) return 101;

#if defined(ARDUINO_ARCH_MEGAAVR)
  if (PWP_TINY_MODE != 0) return 10;
  if (PWP_CORE_MODE != 1) return 11;
  if (caps.memoryTiny) return 12;
  if (std::strcmp(caps.recommendedProfile, "PWP_CORE_MODE") != 0) return 13;
#endif

#if (defined(ARDUINO_ARCH_AVR) || defined(__AVR__)) && !defined(ARDUINO_ARCH_MEGAAVR)
  if (PWP_TINY_MODE != 1) return 20;
  if (!caps.memoryTiny) return 21;
  if (std::strcmp(caps.recommendedProfile, "PWP_TINY_MODE") != 0) return 22;
#endif

#if defined(ARDUINO_ARCH_RENESAS_UNO)
  if (!caps.hasAnalogReadResolution) return 30;
  if (caps.defaultAdcBits != 14) return 31;
#endif

#if defined(ARDUINO_ARCH_ESP8266)
  if (PWP_TINY_MODE != 0) return 40;
  if (PWP_CORE_MODE != 1) return 41;
  if (PWP_FULL_MODE != 0) return 42;
  if (!caps.singleAdcChannel) return 43;
  if (std::strcmp(caps.recommendedProfile, "PWP_CORE_MODE") != 0) return 44;
#endif

  return 0;
}
