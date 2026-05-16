#ifndef PWP_EXAMPLE_PINS_H
#define PWP_EXAMPLE_PINS_H

/* Shared example pins for PulseWetProbe examples.
 * Override these macros from the IDE/CLI or edit this file for your board.
 * These are example defaults only; verify your board pinout and ADC/touch maps.
 */

#ifndef PWP_EXAMPLE_SENSE_PIN
  #if defined(A0)
    #define PWP_EXAMPLE_SENSE_PIN A0
  #elif defined(ARDUINO_ARCH_ESP32)
    #define PWP_EXAMPLE_SENSE_PIN 34
  #elif defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
    #define PWP_EXAMPLE_SENSE_PIN 26
  #else
    #define PWP_EXAMPLE_SENSE_PIN 0
  #endif
#endif

#ifndef PWP_EXAMPLE_EXCITE_PIN
  #if defined(ARDUINO_ARCH_ESP32)
    #define PWP_EXAMPLE_EXCITE_PIN 25
  #elif defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
    #define PWP_EXAMPLE_EXCITE_PIN 15
  #else
    #define PWP_EXAMPLE_EXCITE_PIN 7
  #endif
#endif

#ifndef PWP_EXAMPLE_PLATE_A_PIN
  #define PWP_EXAMPLE_PLATE_A_PIN PWP_EXAMPLE_SENSE_PIN
#endif

#ifndef PWP_EXAMPLE_PLATE_B_PIN
  #if defined(A1)
    #define PWP_EXAMPLE_PLATE_B_PIN A1
  #elif defined(ARDUINO_ARCH_ESP32)
    /* GPIO35 is input-only on classic ESP32. Use GPIO33 which is ADC1-capable
     * and supports both INPUT and OUTPUT, making it safe for reversible mode. */
    #define PWP_EXAMPLE_PLATE_B_PIN 33
  #elif defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
    #define PWP_EXAMPLE_PLATE_B_PIN 27
  #else
    #define PWP_EXAMPLE_PLATE_B_PIN 1
  #endif
#endif


#ifndef PWP_EXAMPLE_REV_A_PIN
  #if defined(ARDUINO_ARCH_ESP32)
    /* For reversible mode both pins must support OUTPUT. GPIO34-39 are input-only
     * on classic ESP32. Use GPIO32/33 (ADC1 ch4/ch5, bidirectional). */
    #define PWP_EXAMPLE_REV_A_PIN 32
  #else
    #define PWP_EXAMPLE_REV_A_PIN PWP_EXAMPLE_PLATE_A_PIN
  #endif
#endif

#ifndef PWP_EXAMPLE_REV_B_PIN
  #if defined(ARDUINO_ARCH_ESP32)
    #define PWP_EXAMPLE_REV_B_PIN 33
  #else
    #define PWP_EXAMPLE_REV_B_PIN PWP_EXAMPLE_PLATE_B_PIN
  #endif
#endif

#ifndef PWP_EXAMPLE_TOUCH_PIN
  // Classic ESP32 often uses GPIO 4 / T0. Always verify the selected ESP32 variant.
  #define PWP_EXAMPLE_TOUCH_PIN 4
#endif

#endif
