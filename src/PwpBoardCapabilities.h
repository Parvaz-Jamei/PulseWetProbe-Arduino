#ifndef PWP_BOARD_CAPABILITIES_H
#define PWP_BOARD_CAPABILITIES_H

#include "PwpConfig.h"
#include <stdint.h>

#if defined(ARDUINO_ARCH_MEGAAVR)
  #define PWP_BOARD_MEGAAVR 1
#else
  #define PWP_BOARD_MEGAAVR 0
#endif

#if (defined(ARDUINO_ARCH_AVR) || defined(__AVR__)) && !defined(ARDUINO_ARCH_MEGAAVR)
  #define PWP_BOARD_AVR 1
#else
  #define PWP_BOARD_AVR 0
#endif

#if defined(ARDUINO_ARCH_ESP32)
  #define PWP_BOARD_ESP32 1
#else
  #define PWP_BOARD_ESP32 0
#endif

#if defined(ARDUINO_ARCH_ESP8266)
  #define PWP_BOARD_ESP8266 1
#else
  #define PWP_BOARD_ESP8266 0
#endif

#if defined(ARDUINO_ARCH_SAMD)
  #define PWP_BOARD_SAMD 1
#else
  #define PWP_BOARD_SAMD 0
#endif

#if defined(ARDUINO_ARCH_RENESAS) || defined(ARDUINO_ARCH_RENESAS_UNO)
  #define PWP_BOARD_RENESAS 1
#else
  #define PWP_BOARD_RENESAS 0
#endif

/* MBED_RP2040 must be checked before generic MBED because Arduino Mbed Core
 * may define both ARDUINO_ARCH_MBED and ARDUINO_ARCH_MBED_RP2040 on the
 * Nano RP2040 Connect. The more-specific board wins. */
#if defined(ARDUINO_ARCH_MBED_RP2040)
  #define PWP_BOARD_MBED_RP2040_SPECIFIC 1
#else
  #define PWP_BOARD_MBED_RP2040_SPECIFIC 0
#endif

#if (defined(ARDUINO_ARCH_MBED) || defined(ARDUINO_ARCH_MBED_NANO) || defined(ARDUINO_ARCH_NRF52)) && !PWP_BOARD_MBED_RP2040_SPECIFIC
  #define PWP_BOARD_MBED_OR_NRF52 1
#else
  #define PWP_BOARD_MBED_OR_NRF52 0
#endif

#if (defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040))
  #define PWP_BOARD_RP2040 1
#else
  #define PWP_BOARD_RP2040 0
#endif

#if defined(ARDUINO_ARCH_STM32) || defined(STM32_CORE_VERSION)
  #define PWP_BOARD_STM32 1
#else
  #define PWP_BOARD_STM32 0
#endif

#if defined(TEENSYDUINO) || defined(CORE_TEENSY)
  #define PWP_BOARD_TEENSY 1
#else
  #define PWP_BOARD_TEENSY 0
#endif

struct PwpBoardCapabilities {
  bool hasAnalogRead;
  bool hasAnalogReadResolution;
  bool hasInternalPullup;
  bool hasNativeTouch;
  bool hasDeepSleepFriendlyHooks;
  bool memoryTiny;
  bool supportsJsonOutput;
  bool supportsFaultLog;
  bool supportsAdvancedFilters;
  bool supportsHampelFilter;
  bool supportsProfileStorage;
  bool supportsMultiPointCalibration;
  bool singleAdcChannel;
  bool fourPlateRecommended;
  uint8_t defaultAdcBits;
  uint8_t maxAdcBits;
  uint8_t recommendedBurstSamples;
  const char* recommendedProfile;
  const char* family;
  const char* notes;
};

PwpBoardCapabilities pwpDetectBoardCapabilities();

#endif
