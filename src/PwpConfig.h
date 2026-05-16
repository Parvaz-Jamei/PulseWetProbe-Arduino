#ifndef PWP_CONFIG_H
#define PWP_CONFIG_H

#define PWP_VERSION_MAJOR 0
#define PWP_VERSION_MINOR 3
#define PWP_VERSION_PATCH 7
#define PWP_VERSION "0.3.7"

/*
 * PulseWetProbe compile profile policy:
 * - Classic AVR keeps a small fixed-memory Tiny profile by default.
 * - megaAVR and ESP8266 use Core-mode defaults to stay memory-conservative.
 * - Other non-AVR boards get Core + Full-capable code paths by default.
 * - JSON, heap-backed String output, fault log, Hampel and profile storage
 *   remain off on AVR/Tiny unless the user opts in.
 */
#if defined(ARDUINO_ARCH_MEGAAVR)
  /* megaAVR (Nano Every / Uno WiFi Rev2 class) may also expose __AVR__.
   * Detect it before classic AVR so it receives Core-mode defaults instead of
   * being forced into the smallest Tiny profile.
   */
  #ifndef PWP_TINY_MODE
    #define PWP_TINY_MODE 0
  #endif
  #ifndef PWP_CORE_MODE
    #define PWP_CORE_MODE 1
  #endif
  #ifndef PWP_FULL_MODE
    #define PWP_FULL_MODE 0
  #endif
#elif (defined(ARDUINO_ARCH_AVR) || defined(__AVR__)) && !defined(ARDUINO_ARCH_MEGAAVR)
  #ifndef PWP_TINY_MODE
    #define PWP_TINY_MODE 1
  #endif
  #ifndef PWP_CORE_MODE
    #define PWP_CORE_MODE 0
  #endif
  #ifndef PWP_FULL_MODE
    #define PWP_FULL_MODE 0
  #endif
#elif defined(ARDUINO_ARCH_ESP8266)
  /* ESP8266 is memory constrained and exposes a single user ADC channel.
   * Keep it in Core mode by default so docs, capabilities and compiled feature
   * set stay aligned.
   */
  #ifndef PWP_TINY_MODE
    #define PWP_TINY_MODE 0
  #endif
  #ifndef PWP_CORE_MODE
    #define PWP_CORE_MODE 1
  #endif
  #ifndef PWP_FULL_MODE
    #define PWP_FULL_MODE 0
  #endif
#else
  #ifndef PWP_TINY_MODE
    #define PWP_TINY_MODE 0
  #endif
  #ifndef PWP_CORE_MODE
    #define PWP_CORE_MODE 1
  #endif
  #ifndef PWP_FULL_MODE
    #define PWP_FULL_MODE 1
  #endif
#endif

#if defined(ARDUINO_ARCH_ESP32)
  #ifndef PWP_ESP32_MODE
    #define PWP_ESP32_MODE 1
  #endif
#else
  #ifndef PWP_ESP32_MODE
    #define PWP_ESP32_MODE 0
  #endif
#endif


#ifndef PWP_ENABLE_JSON
  #if PWP_TINY_MODE
    #define PWP_ENABLE_JSON 0
  #else
    #define PWP_ENABLE_JSON 1
  #endif
#endif

/* Heap-backed Arduino String convenience output is disabled by default in Tiny
 * mode. Buffer-based toCsv(char*, size_t) / toJson(char*, size_t) remains the
 * preferred industrial path on small AVR boards.
 */
#ifndef PWP_ENABLE_STRING_OUTPUT
  #if PWP_TINY_MODE
    #define PWP_ENABLE_STRING_OUTPUT 0
  #else
    #define PWP_ENABLE_STRING_OUTPUT 1
  #endif
#endif

#ifndef PWP_ENABLE_FAULT_LOG
  #if PWP_TINY_MODE
    #define PWP_ENABLE_FAULT_LOG 0
  #else
    #define PWP_ENABLE_FAULT_LOG 1
  #endif
#endif

#ifndef PWP_ENABLE_HAMPEL_FILTER
  #if PWP_FULL_MODE && !PWP_TINY_MODE
    #define PWP_ENABLE_HAMPEL_FILTER 1
  #else
    #define PWP_ENABLE_HAMPEL_FILTER 0
  #endif
#endif

#ifndef PWP_ENABLE_PROFILE_STORAGE
  #if PWP_FULL_MODE && !PWP_TINY_MODE
    #define PWP_ENABLE_PROFILE_STORAGE 1
  #else
    #define PWP_ENABLE_PROFILE_STORAGE 0
  #endif
#endif

#ifndef PWP_ENABLE_POWER_SCHEDULER
  #define PWP_ENABLE_POWER_SCHEDULER 1
#endif

/* ESP32 touch is compiled only when the Arduino-ESP32 core exposes touch sensor support. */
#if defined(ARDUINO_ARCH_ESP32) && defined(SOC_TOUCH_SENSOR_SUPPORTED) && SOC_TOUCH_SENSOR_SUPPORTED
  #ifndef PWP_COMPILE_ESP32_TOUCH
    #define PWP_COMPILE_ESP32_TOUCH 1
  #endif
#else
  #ifndef PWP_COMPILE_ESP32_TOUCH
    #define PWP_COMPILE_ESP32_TOUCH 0
  #endif
#endif

#ifndef PWP_ENABLE_ESP32_TOUCH
  #if PWP_ESP32_MODE && PWP_COMPILE_ESP32_TOUCH
    #define PWP_ENABLE_ESP32_TOUCH 1
  #else
    #define PWP_ENABLE_ESP32_TOUCH 0
  #endif
#endif

#if PWP_TINY_MODE
  #define PWP_MAX_BURST_SAMPLES 8
  #define PWP_MEDIAN_WINDOW 3
  #define PWP_MOVING_AVG_WINDOW 4
  #define PWP_HAMPEL_WINDOW 0
  #define PWP_MAX_CAL_POINTS 2
  #define PWP_FAULT_LOG_CAPACITY 0
  /* Catch dangerous opt-in combinations on Tiny mode early. */
  #if PWP_ENABLE_HAMPEL_FILTER
    #error "PWP_ENABLE_HAMPEL_FILTER=1 is not supported on Tiny/AVR (PWP_HAMPEL_WINDOW=0 would create a zero-size array). Disable Tiny mode or do not enable Hampel on this platform."
  #endif
  #if PWP_ENABLE_FAULT_LOG
    #error "PWP_ENABLE_FAULT_LOG=1 is not supported on Tiny/AVR (PWP_FAULT_LOG_CAPACITY=0 would create a zero-size array). Disable Tiny mode or do not enable the fault log on this platform."
  #endif
#else
  #define PWP_MAX_BURST_SAMPLES 32
  #define PWP_MEDIAN_WINDOW 5
  #define PWP_MOVING_AVG_WINDOW 8
  #define PWP_HAMPEL_WINDOW 5
  #define PWP_MAX_CAL_POINTS 5
  #define PWP_FAULT_LOG_CAPACITY 6
#endif

#define PWP_MIN_ADC_BITS 8
#define PWP_MAX_ADC_BITS 16
#define PWP_DEFAULT_ADC_BITS 10
#define PWP_DEFAULT_SETTLING_US 80
#define PWP_DEFAULT_DUMMY_READS 2
#define PWP_DEFAULT_BURST_SAMPLES 8
#define PWP_DEFAULT_INTERVAL_MS 1000UL

#ifndef PWP_CSV_BUFFER_SIZE
  #define PWP_CSV_BUFFER_SIZE 384
#endif

#ifndef PWP_JSON_BUFFER_SIZE
  #define PWP_JSON_BUFFER_SIZE 512
#endif

#ifndef PWP_FLAGS_BUFFER_SIZE
  #define PWP_FLAGS_BUFFER_SIZE 192
#endif

#ifndef PWP_SERIAL_TIMEOUT_MS
  #define PWP_SERIAL_TIMEOUT_MS 3000UL
#endif
#define PWP_PROFILE_MAGIC 0x50575032UL

#ifndef PWP_CONDUCTIVITY_LEVEL_WEIGHT_DEFAULT
  #define PWP_CONDUCTIVITY_LEVEL_WEIGHT_DEFAULT 0.70f
#endif
#ifndef PWP_CONDUCTIVITY_DIFF_WEIGHT_DEFAULT
  #define PWP_CONDUCTIVITY_DIFF_WEIGHT_DEFAULT 0.30f
#endif
#ifndef PWP_CONDUCTIVITY_LEVEL_WEIGHT_CONDUCTIVITY_PROFILE
  #define PWP_CONDUCTIVITY_LEVEL_WEIGHT_CONDUCTIVITY_PROFILE 0.45f
#endif
#ifndef PWP_CONDUCTIVITY_DIFF_WEIGHT_CONDUCTIVITY_PROFILE
  #define PWP_CONDUCTIVITY_DIFF_WEIGHT_CONDUCTIVITY_PROFILE 0.55f
#endif
#ifndef PWP_CONDUCTIVITY_LEVEL_WEIGHT_REVERSIBLE
  #define PWP_CONDUCTIVITY_LEVEL_WEIGHT_REVERSIBLE 0.55f
#endif
#ifndef PWP_CONDUCTIVITY_DIFF_WEIGHT_REVERSIBLE
  #define PWP_CONDUCTIVITY_DIFF_WEIGHT_REVERSIBLE 0.45f
#endif
#ifndef PWP_CONDUCTIVITY_TREND_EMA_ALPHA
  #define PWP_CONDUCTIVITY_TREND_EMA_ALPHA 0.15f
#endif
#ifndef PWP_CONDUCTIVITY_TREND_GAIN
  #define PWP_CONDUCTIVITY_TREND_GAIN 4.0f
#endif

#ifndef PWP_THRESHOLD_DRY
  #define PWP_THRESHOLD_DRY 0.25f
#endif
#ifndef PWP_THRESHOLD_DAMP
  #define PWP_THRESHOLD_DAMP 0.55f
#endif
#ifndef PWP_THRESHOLD_WET
  #define PWP_THRESHOLD_WET 0.85f
#endif
#ifndef PWP_NOISE_UNSTABLE_TINY
  #define PWP_NOISE_UNSTABLE_TINY 160
#endif
#ifndef PWP_NOISE_UNSTABLE_CORE
  #define PWP_NOISE_UNSTABLE_CORE 240
#endif

#ifndef PWP_DIAGNOSTIC_NOISE_UNSTABLE_TINY
  #define PWP_DIAGNOSTIC_NOISE_UNSTABLE_TINY 120
#endif
#ifndef PWP_DIAGNOSTIC_NOISE_UNSTABLE_CORE
  #define PWP_DIAGNOSTIC_NOISE_UNSTABLE_CORE 180
#endif
#ifndef PWP_CABLE_NOISE_TINY
  #define PWP_CABLE_NOISE_TINY 80
#endif
#ifndef PWP_CABLE_NOISE_CORE
  #define PWP_CABLE_NOISE_CORE 130
#endif

#ifndef PWP_FOULING_WEIGHT_POLARITY_IMBALANCE
  #define PWP_FOULING_WEIGHT_POLARITY_IMBALANCE 35
#endif
#ifndef PWP_FOULING_WEIGHT_STUCK_READING
  #define PWP_FOULING_WEIGHT_STUCK_READING 25
#endif
#ifndef PWP_FOULING_WEIGHT_DRIFT
  #define PWP_FOULING_WEIGHT_DRIFT 20
#endif
#ifndef PWP_FOULING_WEIGHT_CABLE_NOISE
  #define PWP_FOULING_WEIGHT_CABLE_NOISE 15
#endif
#ifndef PWP_FOULING_WEIGHT_UNSTABLE
  #define PWP_FOULING_WEIGHT_UNSTABLE 10
#endif
#ifndef PWP_FOULING_WEIGHT_CORROSION_RISK
  #define PWP_FOULING_WEIGHT_CORROSION_RISK 20
#endif
#ifndef PWP_FOULING_WEIGHT_WIRING_SUSPECT
  #define PWP_FOULING_WEIGHT_WIRING_SUSPECT 10
#endif



#ifndef PWP_FILTER_NOISE_ADAPT_THRESHOLD
  #define PWP_FILTER_NOISE_ADAPT_THRESHOLD 80
#endif
#ifndef PWP_FILTER_EMA_NOISE_DIVISOR
  #define PWP_FILTER_EMA_NOISE_DIVISOR 40
#endif
#ifndef PWP_FILTER_EMA_MAX_REDUCTION
  #define PWP_FILTER_EMA_MAX_REDUCTION 25
#endif
#ifndef PWP_FILTER_MIN_EMA_ALPHA
  #define PWP_FILTER_MIN_EMA_ALPHA 5
#endif
#ifndef PWP_OUTLIER_NOISE_DIVISOR
  #define PWP_OUTLIER_NOISE_DIVISOR 16
#endif
#ifndef PWP_OUTLIER_MIN_LIMIT
  #define PWP_OUTLIER_MIN_LIMIT 8
#endif
#ifndef PWP_POLARIZATION_ASYMMETRY_DIVISOR
  #define PWP_POLARIZATION_ASYMMETRY_DIVISOR 20L
#endif
#ifndef PWP_POLARIZATION_ASYMMETRY_MIN
  #define PWP_POLARIZATION_ASYMMETRY_MIN 24
#endif

#ifndef PWP_CHARGE_BALANCE_THRESHOLD
  #define PWP_CHARGE_BALANCE_THRESHOLD 8
#endif
#ifndef PWP_STUCK_READING_THRESHOLD
  #define PWP_STUCK_READING_THRESHOLD 12
#endif
#ifndef PWP_STUCK_SCHEDULER_THRESHOLD
  #define PWP_STUCK_SCHEDULER_THRESHOLD 8
#endif

#ifndef PWP_DEFAULT_INTER_SAMPLE_DELAY_US
  #define PWP_DEFAULT_INTER_SAMPLE_DELAY_US 20
#endif
#ifndef PWP_INTER_SAMPLE_DELAY_TINY_US
  #define PWP_INTER_SAMPLE_DELAY_TINY_US 30
#endif
#ifndef PWP_INTER_SAMPLE_DELAY_CORE_US
  #define PWP_INTER_SAMPLE_DELAY_CORE_US 25
#endif
#ifndef PWP_INTER_SAMPLE_DELAY_FULL_US
  #define PWP_INTER_SAMPLE_DELAY_FULL_US 15
#endif
#ifndef PWP_INTER_SAMPLE_DELAY_ESP32_US
  #define PWP_INTER_SAMPLE_DELAY_ESP32_US PWP_DEFAULT_INTER_SAMPLE_DELAY_US
#endif

#ifndef PWP_QUALITY_PENALTY_NEEDS_CALIBRATION
  #define PWP_QUALITY_PENALTY_NEEDS_CALIBRATION 20
#endif
#ifndef PWP_QUALITY_PENALTY_UNSTABLE
  #define PWP_QUALITY_PENALTY_UNSTABLE 25
#endif
#ifndef PWP_QUALITY_PENALTY_LOW_CONFIDENCE
  #define PWP_QUALITY_PENALTY_LOW_CONFIDENCE 10
#endif
#ifndef PWP_QUALITY_PENALTY_SHORT_OPEN
  #define PWP_QUALITY_PENALTY_SHORT_OPEN 45
#endif
#ifndef PWP_QUALITY_PENALTY_POSSIBLE_FAULT
  #define PWP_QUALITY_PENALTY_POSSIBLE_FAULT 10
#endif
#ifndef PWP_QUALITY_PENALTY_CORROSION
  #define PWP_QUALITY_PENALTY_CORROSION 10
#endif
#ifndef PWP_QUALITY_PENALTY_CABLE_NOISE
  #define PWP_QUALITY_PENALTY_CABLE_NOISE 12
#endif
#ifndef PWP_QUALITY_PENALTY_STUCK
  #define PWP_QUALITY_PENALTY_STUCK 15
#endif
#ifndef PWP_QUALITY_PENALTY_DRIFT
  #define PWP_QUALITY_PENALTY_DRIFT 10
#endif
#ifndef PWP_QUALITY_PENALTY_TOUCH_UNSUPPORTED
  #define PWP_QUALITY_PENALTY_TOUCH_UNSUPPORTED 40
#endif
#ifndef PWP_QUALITY_NOISE_DIVISOR
  #define PWP_QUALITY_NOISE_DIVISOR 25
#endif
#ifndef PWP_QUALITY_NOISE_MAX_PENALTY
  #define PWP_QUALITY_NOISE_MAX_PENALTY 25
#endif

#ifndef PWP_FAULT_CONFIRM_WINDOWS
  #define PWP_FAULT_CONFIRM_WINDOWS 3
#endif

#endif
