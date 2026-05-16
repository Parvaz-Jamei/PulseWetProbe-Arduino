#ifndef PULSE_WET_PROBE_H
#define PULSE_WET_PROBE_H

#if defined(ARDUINO)
  #include <Arduino.h>
  #if defined(ARDUINO_ARCH_ESP32)
    #ifdef __has_include
      #if __has_include(<soc/soc_caps.h>)
        #include <soc/soc_caps.h>
      #endif
    #endif
  #endif
#else
  #include <stdint.h>
  #include <stddef.h>
  #include <stdio.h>
  #include <string>
  typedef std::string String;
  #ifndef HIGH
    #define HIGH 0x1
  #endif
  #ifndef LOW
    #define LOW 0x0
  #endif
  #ifndef INPUT
    #define INPUT 0x0
  #endif
  #ifndef OUTPUT
    #define OUTPUT 0x1
  #endif
  #ifndef INPUT_PULLUP
    #define INPUT_PULLUP 0x2
  #endif
#endif

#include "PwpConfig.h"
#include "PwpBoardCapabilities.h"

enum class PwpProfile : uint8_t {
  RAW = 0,
  SOIL = 1,
  WETNESS = 2,
  CONDUCTIVITY_TREND = 3,
  SURFACE_BRINE_TREND = 4,
  DIAGNOSTIC = 5
};

enum class PwpExcitation : uint8_t {
  // Backward-compatible name. This is a high/low pulse on one drive
  // electrode, not true electrode role reversal. Prefer PULSED_HIGH_LOW
  // in new code, or REVERSIBLE_TWO_PLATE for balanced two-pin sampling.
  PULSED_HIGH_LOW = 0,
  PULSED_ALTERNATING = PULSED_HIGH_LOW,
  PULSED_HIGH_ONLY = 1,
  PULLUP_WETNESS = 2,
  ESP32_TOUCH = 3,
  REVERSIBLE_TWO_PLATE = 4
};

enum class PwpFilterPreset : uint8_t {
  RAW = 0,
  RESPONSIVE = 1,
  STABLE = 2,
  INDUSTRIAL_STABLE = 3,
  OUTLIER_REJECTING = 4,
  DIAGNOSTIC_RAW = 5
};

enum class PwpReadingState : uint8_t {
  UNKNOWN = 0,
  DRY = 1,
  DAMP = 2,
  WET = 3,
  SATURATED = 4,
  UNSTABLE = 5,
  NEEDS_CALIBRATION = 6
};

const char* pwpReadingStateName(PwpReadingState state);

enum class PwpBeginStatus : uint8_t {
  OK = 0,
  INVALID_PIN = 1,
  TOUCH_NOT_COMPILED = 2,
  TOUCH_UNSUPPORTED_BOARD = 3,
  TOUCH_UNSUPPORTED_PIN = 4
};

struct PwpDiagnostics {
  bool openCircuit;
  bool shortCircuit;
  bool possibleOpen;
  bool possibleShort;
  bool unstable;
  bool needsCalibration;
  bool lowConfidence;
  bool corrosionRisk;
  bool saturationLow;
  bool saturationHigh;
  bool cableNoiseSuspected;
  bool stuckReading;
  bool driftSuspected;
  bool touchUnsupported;
  uint16_t noiseScore;
  uint16_t stabilityScore;
  uint16_t driftScore;
  uint16_t foulingScore;

  PwpDiagnostics();
  bool ok() const;
  void toFlagString(char* out, size_t outSize) const;
};

struct PwpReading {
  uint32_t sequence;
  unsigned long timestampMillis;
  int rawForward;
  int rawReverse;
  int rawDifferential;
  int rawAverage;
  int filtered;
  int touchRaw;
  uint16_t burstSpread;
  float normalizedWetness;
  float levelIndex;
  float differentialIndex;
  float conductivityTrend;
  uint8_t qualityScore;
  PwpReadingState state;
  PwpDiagnostics diagnostics;

  PwpReading();
  bool isWet() const;
  float wetnessPercent() const;
  const char* stateName() const;
#if PWP_ENABLE_STRING_OUTPUT
  String toCsv() const;
#endif
  size_t toCsv(char* out, size_t outSize) const;
#if PWP_ENABLE_JSON
#if PWP_ENABLE_STRING_OUTPUT
  String toJson() const;
#endif
  size_t toJson(char* out, size_t outSize) const;
#endif
};

struct PwpCalibrationPoint {
  int raw;
  float wetness;
  float temperatureC;
};

struct PwpCalibrationProfile {
  uint32_t magic;
  uint16_t version;
  uint8_t pointCount;
  int dryRaw;
  int wetRaw;
  float referenceTemperatureC;
  float temperatureCoefficientPerC;
  PwpCalibrationPoint points[PWP_MAX_CAL_POINTS];
  uint16_t crc;
};

struct PwpFaultEntry {
  uint32_t sequence;
  unsigned long timestampMillis;
  uint8_t flags;
  uint16_t noiseScore;
  uint8_t qualityScore;
};

class PwpAcquisitionConfig {
public:
  PwpAcquisitionConfig();

  PwpAcquisitionConfig& setSettlingMicros(uint16_t value);
  PwpAcquisitionConfig& setDummyReads(uint8_t value);
  PwpAcquisitionConfig& setBurstSamples(uint8_t value);
  PwpAcquisitionConfig& enableSynchronousSampling(bool enabled);
  PwpAcquisitionConfig& setInterSampleDelayMicros(uint16_t value);
  PwpAcquisitionConfig& enableTrimmedBurstMean(bool enabled);

  uint16_t settlingMicros() const;
  uint8_t dummyReads() const;
  uint8_t burstSamples() const;
  bool synchronousSampling() const;
  uint16_t interSampleDelayMicros() const;
  bool trimmedBurstMean() const;

private:
  uint16_t _settlingMicros;
  uint8_t _dummyReads;
  uint8_t _burstSamples;
  bool _sync;
  uint16_t _interSampleDelayMicros;
  bool _trimmedBurstMean;
};

class PwpFilterConfig {
public:
  PwpFilterConfig();

  PwpFilterConfig& setPreset(PwpFilterPreset preset);
  PwpFilterConfig& setEmaAlpha(uint8_t alphaPercent);
  PwpFilterConfig& setMovingAverageWindow(uint8_t samples);
  PwpFilterConfig& setMedianEnabled(bool enabled);
  PwpFilterConfig& setHysteresis(float enterWet, float exitWet);
  PwpFilterConfig& enableOutlierRejector(bool enabled);
  PwpFilterConfig& setOutlierLimit(uint16_t maxStep);
  PwpFilterConfig& enableHampelFilter(bool enabled);
  PwpFilterConfig& setHampelThreshold(uint16_t threshold);

  PwpFilterPreset preset() const;
  uint8_t emaAlpha() const;
  uint8_t movingAverageWindow() const;
  bool medianEnabled() const;
  float enterWetThreshold() const;
  float exitWetThreshold() const;
  bool outlierRejectorEnabled() const;
  uint16_t outlierLimit() const;
  bool hampelFilterEnabled() const;
  uint16_t hampelThreshold() const;

private:
  PwpFilterPreset _preset;
  uint8_t _emaAlpha;
  uint8_t _movingAverageWindow;
  bool _medianEnabled;
  float _enterWet;
  float _exitWet;
  bool _outlierRejector;
  uint16_t _outlierLimit;
  bool _hampelEnabled;
  uint16_t _hampelThreshold;
};

class PwpCalibrationConfig {
public:
  PwpCalibrationConfig();

  PwpCalibrationConfig& setDryWet(int dryRaw, int wetRaw);
  PwpCalibrationConfig& clear();
  PwpCalibrationConfig& clearPoints();
  bool addPoint(int raw, float wetness);
  bool addPoint(int raw, float wetness, float temperatureC);
  PwpCalibrationConfig& setReferenceTemperatureC(float value);
  PwpCalibrationConfig& setTemperatureCoefficient(float wetnessPerC);
  PwpCalibrationConfig& setCurrentTemperatureC(float value);

  bool calibrated() const;
  int dryRaw() const;
  int wetRaw() const;
  uint8_t pointCount() const;
  PwpCalibrationPoint pointAt(uint8_t index) const;
  float referenceTemperatureC() const;
  float currentTemperatureC() const;
  float temperatureCoefficient() const;
  float normalize(int raw, uint8_t adcBits) const;

  PwpCalibrationProfile exportProfile() const;
  bool importProfile(const PwpCalibrationProfile& profile);
  static uint16_t computeProfileCrc(const PwpCalibrationProfile& profile);

private:
  void sortPoints();
  float clamp01(float value) const;

  bool _calibrated;
  int _dryRaw;
  int _wetRaw;
  uint8_t _pointCount;
  PwpCalibrationPoint _points[PWP_MAX_CAL_POINTS];
  float _referenceTemperatureC;
  float _currentTemperatureC;
  float _temperatureCoefficient;
};

class PulseWetProbe {
public:
  PulseWetProbe();

  bool beginTwoPlate(int sensePin, int excitationPin);
  bool beginTwoPlate(int sensePin, int excitationPin, uint8_t adcBits);
  bool beginReversibleTwoPlate(int plateAPin, int plateBPin);
  bool beginReversibleTwoPlate(int plateAPin, int plateBPin, uint8_t adcBits);
  bool beginBalancedTwoPlate(int plateAPin, int plateBPin);
  bool beginTouch(int touchPin);

  bool beginSoil(int sensePin, int excitationPin);
  bool beginWetDry(int sensePin, int excitationPin);
  bool beginConductivityTrend(int sensePin, int excitationPin);
  bool beginEsp32TouchWetness(int touchPin);
  PwpBeginStatus lastBeginStatus() const;

  PulseWetProbe& setProfile(PwpProfile profile);
  PulseWetProbe& setExcitation(PwpExcitation excitation);
  PulseWetProbe& enableBoardDefaults();
  PulseWetProbe& setBaseIntervalMillis(unsigned long value);
  PulseWetProbe& setAdcBits(uint8_t bits);
  PulseWetProbe& setTemperatureC(float value);
  PulseWetProbe& calibrateDryWet(int dryRaw, int wetRaw);
  PulseWetProbe& setConductivityTrendWeights(float levelWeight, float differentialWeight);
  PulseWetProbe& resetConductivityTrendBaseline();

  PwpAcquisitionConfig& acquisition();
  PwpFilterConfig& filters();
  PwpCalibrationConfig& calibration();

  const PwpAcquisitionConfig& acquisition() const;
  const PwpFilterConfig& filters() const;
  const PwpCalibrationConfig& calibration() const;

  PwpReading read();
  unsigned long nextIntervalMillis() const;
  PwpBoardCapabilities capabilities() const;

  uint32_t sequence() const;
  int lastFiltered() const;
  PwpReadingState state() const;
  bool isBegun() const;
  bool touchActive() const;
  float wetnessPercent() const;
  const char* stateName() const;
  void resetFilters();

#if PWP_ENABLE_FAULT_LOG
  uint8_t faultLogCount() const;
  PwpFaultEntry faultAt(uint8_t index) const;
  void clearFaultLog();
#endif

private:
  struct PwpSampleResult {
    int value;
    uint16_t spread;
  };

  PwpSampleResult sampleWithDrive(uint8_t driveLevel);
  PwpSampleResult sampleBetweenPins(int drivePin, int sensePin, uint8_t driveLevel);
  PwpSampleResult samplePullupWetness();
  PwpSampleResult sampleTouchPad();
  PwpSampleResult acquireBurstAverage(int sensePin);
  int applyFilters(int raw, uint16_t noise);
  int applyOutlierRejector(int raw, uint16_t noise) const;
  int applyMedian(int raw);
  int applyMovingAverage(int raw);
#if PWP_ENABLE_HAMPEL_FILTER
  int applyHampel(int raw);
#endif
  PwpDiagnostics diagnose(const PwpReading& reading, uint16_t noise, int previousFiltered, bool hadPreviousFiltered);
  PwpReadingState classifyState(float levelIndex, uint16_t noise) const;
  float computeConductivityTrend(float levelIndex, float differentialIndex);
  uint8_t computeQuality(const PwpDiagnostics& diagnostics, uint16_t noise) const;
  void recordFaultIfNeeded(const PwpReading& reading);
  void restHiZ();
  void configureSenseInput();
  void applyAnalogResolution();
  long maxAdcValue() const;
  bool supportsTouchRuntime() const;
  bool isTouchPinUsable(int touchPin) const;

  bool _begun;
  bool _touchActive;
  bool _reversibleMode;
  int _sensePin;
  int _excitationPin;
  int _touchPin;
  uint8_t _adcBits;
  PwpProfile _profile;
  PwpExcitation _excitation;
  PwpAcquisitionConfig _acquisition;
  PwpFilterConfig _filters;
  PwpCalibrationConfig _calibration;
  PwpBoardCapabilities _capabilities;
  unsigned long _baseIntervalMillis;
  uint32_t _sequence;
  bool _hasLastFiltered;
  int _lastFiltered;
  int _lastRaw;
  float _lastWetnessPercent;
  uint32_t _chargeBalance;
  PwpReadingState _state;
  int _medianBuffer[PWP_MEDIAN_WINDOW];
  uint8_t _medianCount;
  uint8_t _medianIndex;
  int _movingBuffer[PWP_MOVING_AVG_WINDOW];
  uint8_t _movingCount;
  uint8_t _movingIndex;
#if PWP_ENABLE_HAMPEL_FILTER
  int _hampelBuffer[PWP_HAMPEL_WINDOW];
  uint8_t _hampelCount;
  uint8_t _hampelIndex;
#endif
  bool _hasConductivityTrendBaseline;
  float _conductivityCompositeEma;
  float _conductivityLevelWeight;
  float _conductivityDiffWeight;
  bool _conductivityWeightsCustom;
  uint8_t _stuckCount;
  uint8_t _possibleOpenCount;
  uint8_t _possibleShortCount;
  PwpBeginStatus _lastBeginStatus;
#if PWP_ENABLE_FAULT_LOG
  PwpFaultEntry _faultLog[PWP_FAULT_LOG_CAPACITY];
  uint8_t _faultCount;
  uint8_t _faultIndex;
#endif
};

#endif
