#include "PulseWetProbe.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef ARDUINO
static unsigned long pwpHostNow = 0;
static const int* pwpHostAnalogSequence = 0;
static size_t pwpHostAnalogSequenceCount = 0;
static size_t pwpHostAnalogSequenceIndex = 0;

extern "C" void pwpHostSetAnalogSequence(const int* values, size_t count) {
  pwpHostAnalogSequence = values;
  pwpHostAnalogSequenceCount = count;
  pwpHostAnalogSequenceIndex = 0;
}

extern "C" void pwpHostClearAnalogSequence() {
  pwpHostAnalogSequence = 0;
  pwpHostAnalogSequenceCount = 0;
  pwpHostAnalogSequenceIndex = 0;
}

unsigned long millis() { return pwpHostNow += 10; }
void delayMicroseconds(unsigned int) {}
void pinMode(int, int) {}
void digitalWrite(int, int) {}
int analogRead(int) {
  if (pwpHostAnalogSequence != 0 && pwpHostAnalogSequenceCount > 0) {
    size_t index = pwpHostAnalogSequenceIndex;
    if (index >= pwpHostAnalogSequenceCount) index = pwpHostAnalogSequenceCount - 1;
    int value = pwpHostAnalogSequence[index];
    if (pwpHostAnalogSequenceIndex < pwpHostAnalogSequenceCount) {
      pwpHostAnalogSequenceIndex++;
    }
    return value;
  }
  return 512;
}
uint32_t touchRead(uint8_t) { return 512; }
#endif

static uint8_t pwpClampU8(uint8_t value, uint8_t low, uint8_t high) {
  if (value < low) return low;
  if (value > high) return high;
  return value;
}

static int pwpAbsInt(int value) {
  return value < 0 ? -value : value;
}

static int pwpClampInt(int value, int low, int high) {
  if (value < low) return low;
  if (value > high) return high;
  return value;
}

static float pwpClampFloat(float value, float low, float high) {
  if (value < low) return low;
  if (value > high) return high;
  return value;
}

static uint16_t pwpCrc16CcittUpdate(uint16_t crc, uint8_t data) {
  crc ^= (uint16_t)data << 8;
  for (uint8_t i = 0; i < 8; ++i) {
    if (crc & 0x8000u) {
      crc = (uint16_t)((crc << 1) ^ 0x1021u);
    } else {
      crc = (uint16_t)(crc << 1);
    }
  }
  return crc;
}

static uint16_t pwpCrc16CcittUpdateU16(uint16_t crc, uint16_t value) {
  crc = pwpCrc16CcittUpdate(crc, (uint8_t)((value >> 8) & 0xFFu));
  crc = pwpCrc16CcittUpdate(crc, (uint8_t)(value & 0xFFu));
  return crc;
}

static uint16_t pwpCrc16CcittUpdateU32(uint16_t crc, uint32_t value) {
  crc = pwpCrc16CcittUpdateU16(crc, (uint16_t)((value >> 16) & 0xFFFFu));
  crc = pwpCrc16CcittUpdateU16(crc, (uint16_t)(value & 0xFFFFu));
  return crc;
}

static uint16_t pwpCrc16CcittUpdateI16(uint16_t crc, int value) {
  return pwpCrc16CcittUpdateU16(crc, (uint16_t)value);
}

const char* pwpReadingStateName(PwpReadingState state) {
  switch (state) {
    case PwpReadingState::DRY: return "dry";
    case PwpReadingState::DAMP: return "damp";
    case PwpReadingState::WET: return "wet";
    case PwpReadingState::SATURATED: return "saturated";
    case PwpReadingState::UNSTABLE: return "unstable";
    case PwpReadingState::NEEDS_CALIBRATION: return "needs_calibration";
    case PwpReadingState::UNKNOWN:
    default: return "unknown";
  }
}

PwpBoardCapabilities pwpDetectBoardCapabilities() {
  PwpBoardCapabilities caps;
  caps.hasAnalogRead = true;
  caps.hasAnalogReadResolution = false;
  caps.hasInternalPullup = true;
  caps.hasNativeTouch = false;
  caps.hasDeepSleepFriendlyHooks = false;
  caps.memoryTiny = false;
  caps.supportsJsonOutput = (PWP_ENABLE_JSON != 0);
  caps.supportsFaultLog = (PWP_ENABLE_FAULT_LOG != 0);
  caps.supportsAdvancedFilters = true;
  caps.supportsHampelFilter = (PWP_ENABLE_HAMPEL_FILTER != 0);
  caps.supportsProfileStorage = (PWP_ENABLE_PROFILE_STORAGE != 0);
  caps.supportsMultiPointCalibration = (PWP_MAX_CAL_POINTS >= 3);
  caps.singleAdcChannel = false;
  caps.fourPlateRecommended = false;
  caps.defaultAdcBits = PWP_DEFAULT_ADC_BITS;
  caps.maxAdcBits = 10;
  caps.recommendedBurstSamples = PWP_DEFAULT_BURST_SAMPLES;
  caps.recommendedProfile = "PWP_CORE_MODE";
  caps.family = "generic-arduino";
  caps.notes = "Generic Arduino-compatible board. Uses conservative 10-bit-compatible defaults.";

#if PWP_BOARD_MEGAAVR
  caps.defaultAdcBits = 10;
  caps.maxAdcBits = 12;
  caps.recommendedBurstSamples = 8;
  caps.recommendedProfile = "PWP_CORE_MODE";
  caps.family = "megaavr";
  caps.notes = "megaAVR Core mode: conservative memory, extra diagnostics where safe.";
#elif PWP_BOARD_AVR
  caps.memoryTiny = true;
  caps.supportsAdvancedFilters = false;
  caps.supportsHampelFilter = false;
  caps.supportsJsonOutput = false;
  caps.supportsFaultLog = false;
  caps.supportsProfileStorage = false;
  caps.supportsMultiPointCalibration = false;
  caps.defaultAdcBits = 10;
  caps.maxAdcBits = 10;
  caps.recommendedBurstSamples = 5;
  caps.recommendedProfile = "PWP_TINY_MODE";
  caps.family = "avr";
  caps.notes = "AVR Tiny mode: fixed buffers, CSV-first output, no default JSON/Hampel/fault-log.";
#elif PWP_BOARD_ESP32
  caps.hasAnalogReadResolution = true;
  caps.hasNativeTouch = (PWP_ENABLE_ESP32_TOUCH != 0);
  caps.hasDeepSleepFriendlyHooks = true;
  caps.defaultAdcBits = 12;
  caps.maxAdcBits = 12;
  caps.recommendedBurstSamples = 16;
  caps.recommendedProfile = "PWP_ESP32_MODE";
  caps.family = "esp32";
  caps.notes = caps.hasNativeTouch ? "ESP32 mode: ADC profile plus guarded native touchRead support." : "ESP32 mode: ADC profile; native touch disabled because this target/core did not expose touch support.";
#elif PWP_BOARD_ESP8266
  caps.defaultAdcBits = 10;
  caps.maxAdcBits = 10;
  caps.recommendedBurstSamples = 8;
  caps.supportsAdvancedFilters = false;
  caps.supportsHampelFilter = false;
  caps.hasDeepSleepFriendlyHooks = true;
  caps.singleAdcChannel = true;
  caps.fourPlateRecommended = false;
  caps.recommendedProfile = "PWP_CORE_MODE";
  caps.family = "esp8266";
  caps.notes = "ESP8266 has one user ADC channel; multi-channel/four-plate examples must stay disabled.";
#elif PWP_BOARD_SAMD
  caps.hasAnalogReadResolution = true;
  caps.defaultAdcBits = 12;
  caps.maxAdcBits = 12;
  caps.recommendedBurstSamples = 16;
  caps.fourPlateRecommended = true;
  caps.recommendedProfile = "PWP_FULL_MODE";
  caps.family = "samd";
  caps.notes = "SAMD Full mode: analogReadResolution-aware normalization and advanced filters.";
#elif PWP_BOARD_RENESAS
  caps.hasAnalogReadResolution = true;
  caps.defaultAdcBits = 14;
  caps.maxAdcBits = 14;
  caps.recommendedBurstSamples = 12;
  caps.fourPlateRecommended = true;
  caps.recommendedProfile = "PWP_FULL_MODE";
  caps.family = "renesas";
  caps.notes = "Renesas/UNO R4 profile: higher-resolution ADC path; ESP32 radio module is not used for sensing.";
#elif PWP_BOARD_MBED_OR_NRF52
  caps.hasAnalogReadResolution = true;
  caps.defaultAdcBits = 12;
  caps.maxAdcBits = 12;
  caps.recommendedBurstSamples = 16;
  caps.fourPlateRecommended = true;
  caps.recommendedProfile = "PWP_FULL_MODE";
  caps.family = "mbed-nrf52";
  caps.notes = "Mbed/nRF52 Full mode: robust filtering and calibration without mandatory BLE dependencies.";
#elif PWP_BOARD_RP2040
  caps.hasAnalogReadResolution = true;
  caps.defaultAdcBits = 12;
  caps.maxAdcBits = 12;
  caps.recommendedBurstSamples = 16;
  caps.fourPlateRecommended = true;
  caps.recommendedProfile = "PWP_FULL_MODE";
  caps.family = "rp2040";
  caps.notes = "RP2040/RP2350 profile: 12-bit ADC-oriented defaults, no native touch claim.";
#elif PWP_BOARD_STM32
  caps.hasAnalogReadResolution = true;
  caps.defaultAdcBits = 12;
  caps.maxAdcBits = 16;
  caps.recommendedBurstSamples = 16;
  caps.fourPlateRecommended = true;
  caps.recommendedProfile = "PWP_FULL_MODE";
  caps.family = "stm32";
  caps.notes = "STM32 profile: ADC/oversampling-friendly defaults without mandatory STM32 low-level APIs.";
#elif PWP_BOARD_TEENSY
  caps.hasAnalogReadResolution = true;
  caps.defaultAdcBits = 12;
  caps.maxAdcBits = 16;
  caps.recommendedBurstSamples = 16;
  caps.fourPlateRecommended = true;
  caps.recommendedProfile = "PWP_FULL_MODE";
  caps.family = "teensy";
  caps.notes = "Teensy profile: high-rate capable defaults while keeping generic Arduino APIs.";
#endif

  return caps;
}

PwpDiagnostics::PwpDiagnostics()
  : openCircuit(false), shortCircuit(false), possibleOpen(false), possibleShort(false),
    unstable(false), needsCalibration(false), lowConfidence(false), corrosionRisk(false),
    saturationLow(false), saturationHigh(false), cableNoiseSuspected(false),
    stuckReading(false), driftSuspected(false), touchUnsupported(false),
    noiseScore(0), stabilityScore(100), driftScore(0), foulingScore(0) {}

bool PwpDiagnostics::ok() const {
  return !openCircuit && !shortCircuit && !possibleOpen && !possibleShort &&
         !unstable && !needsCalibration && !lowConfidence && !corrosionRisk &&
         !saturationLow && !saturationHigh && !cableNoiseSuspected &&
         !stuckReading && !driftSuspected && !touchUnsupported;
}

void PwpDiagnostics::toFlagString(char* out, size_t outSize) const {
  if (out == 0 || outSize == 0) return;
  out[0] = '\0';
  if (ok()) {
    snprintf(out, outSize, "ok");
    return;
  }
  bool first = true;
  struct FlagPair { bool enabled; const char* label; } flags[] = {
    {openCircuit, "open"}, {shortCircuit, "short"},
    {possibleOpen, "possible_open"}, {possibleShort, "possible_short"},
    {unstable, "unstable"}, {needsCalibration, "needs_cal"}, {lowConfidence, "low_conf"},
    {corrosionRisk, "corrosion"}, {saturationLow, "sat_low"}, {saturationHigh, "sat_high"},
    {cableNoiseSuspected, "cable_noise"}, {stuckReading, "stuck"},
    {driftSuspected, "drift"}, {touchUnsupported, "touch_unsupported"}
  };
  for (size_t i = 0; i < sizeof(flags) / sizeof(flags[0]); ++i) {
    if (!flags[i].enabled) continue;
    size_t used = strlen(out);
    if (used >= outSize - 1) return;
    snprintf(out + used, outSize - used, "%s%s", first ? "" : "|", flags[i].label);
    first = false;
  }
}

PwpReading::PwpReading()
  : sequence(0), timestampMillis(0), rawForward(0), rawReverse(0), rawDifferential(0),
    rawAverage(0), filtered(0), touchRaw(-1), burstSpread(0), normalizedWetness(0.0f),
    levelIndex(0.0f), differentialIndex(0.0f), conductivityTrend(0.0f), qualityScore(0),
    state(PwpReadingState::UNKNOWN), diagnostics() {}

bool PwpReading::isWet() const {
  return state == PwpReadingState::DAMP || state == PwpReadingState::WET ||
         state == PwpReadingState::SATURATED;
}

float PwpReading::wetnessPercent() const {
  return normalizedWetness * 100.0f;
}

const char* PwpReading::stateName() const {
  return pwpReadingStateName(state);
}

size_t PwpReading::toCsv(char* out, size_t outSize) const {
  if (out == 0 || outSize == 0) return 0;
  char flags[PWP_FLAGS_BUFFER_SIZE];
  diagnostics.toFlagString(flags, sizeof(flags));
  int wetPermille = (int)(normalizedWetness * 1000.0f + 0.5f);
  int levelPermille = (int)(levelIndex * 1000.0f + 0.5f);
  int diffPermille = (int)(differentialIndex * 1000.0f + 0.5f);
  int trendPermille = (int)(conductivityTrend * 1000.0f + 0.5f);
  const char* stateLabel = stateName();
  int written = snprintf(out, outSize,
      "%lu,%lu,%d,%d,%d,%d,%d,%d,%u,%d,%d,%d,%d,%u,%u,%u,%u,%u,%s,%s",
      (unsigned long)sequence,
      (unsigned long)timestampMillis,
      rawForward,
      rawReverse,
      rawDifferential,
      rawAverage,
      filtered,
      touchRaw,
      (unsigned int)burstSpread,
      wetPermille,
      levelPermille,
      diffPermille,
      trendPermille,
      (unsigned int)qualityScore,
      (unsigned int)diagnostics.noiseScore,
      (unsigned int)diagnostics.stabilityScore,
      (unsigned int)diagnostics.driftScore,
      (unsigned int)diagnostics.foulingScore,
      stateLabel,
      flags);
  if (written < 0) {
    out[0] = '\0';
    return 0;
  }
  if ((size_t)written >= outSize) {
    // Truncation: zero the buffer so the caller cannot accidentally print
    // partial CSV data (field count would be wrong and silently corrupt logs).
    out[0] = '\0';
    return 0;
  }
  return (size_t)written;
}

#if PWP_ENABLE_STRING_OUTPUT
String PwpReading::toCsv() const {
  char buffer[PWP_CSV_BUFFER_SIZE];
  size_t written = toCsv(buffer, sizeof(buffer));
  if (written == 0) return String("csv_truncated");
  return String(buffer);
}
#endif

#if PWP_ENABLE_JSON
size_t PwpReading::toJson(char* out, size_t outSize) const {
  if (out == 0 || outSize == 0) return 0;
  char flags[PWP_FLAGS_BUFFER_SIZE];
  diagnostics.toFlagString(flags, sizeof(flags));
  const char* stateLabel = stateName();
  int wetPermille = (int)(normalizedWetness * 1000.0f + 0.5f);
  int levelPermille = (int)(levelIndex * 1000.0f + 0.5f);
  int diffPermille = (int)(differentialIndex * 1000.0f + 0.5f);
  int trendPermille = (int)(conductivityTrend * 1000.0f + 0.5f);
  int written = snprintf(out, outSize,
      "{\"seq\":%lu,\"ms\":%lu,\"rawForward\":%d,\"rawReverse\":%d,\"rawDiff\":%d,\"rawAvg\":%d,\"filtered\":%d,\"touchRaw\":%d,\"burstSpread\":%u,\"wetPermille\":%d,\"levelPermille\":%d,\"diffPermille\":%d,\"trendPermille\":%d,\"quality\":%u,\"noise\":%u,\"stability\":%u,\"drift\":%u,\"fouling\":%u,\"state\":\"%s\",\"flags\":\"%s\"}",
      (unsigned long)sequence,
      (unsigned long)timestampMillis,
      rawForward,
      rawReverse,
      rawDifferential,
      rawAverage,
      filtered,
      touchRaw,
      (unsigned int)burstSpread,
      wetPermille,
      levelPermille,
      diffPermille,
      trendPermille,
      (unsigned int)qualityScore,
      (unsigned int)diagnostics.noiseScore,
      (unsigned int)diagnostics.stabilityScore,
      (unsigned int)diagnostics.driftScore,
      (unsigned int)diagnostics.foulingScore,
      stateLabel,
      flags);
  if (written < 0) {
    out[0] = '\0';
    return 0;
  }
  if ((size_t)written >= outSize) {
    out[0] = '\0';
    return 0;
  }
  return (size_t)written;
}

#if PWP_ENABLE_STRING_OUTPUT
String PwpReading::toJson() const {
  char buffer[PWP_JSON_BUFFER_SIZE];
  size_t written = toJson(buffer, sizeof(buffer));
  if (written == 0) return String("json_truncated");
  return String(buffer);
}
#endif
#endif

PwpAcquisitionConfig::PwpAcquisitionConfig()
  : _settlingMicros(PWP_DEFAULT_SETTLING_US), _dummyReads(PWP_DEFAULT_DUMMY_READS),
    _burstSamples(PWP_DEFAULT_BURST_SAMPLES), _sync(true), _interSampleDelayMicros(PWP_DEFAULT_INTER_SAMPLE_DELAY_US),
    _trimmedBurstMean(PWP_FULL_MODE && !PWP_TINY_MODE) {}

PwpAcquisitionConfig& PwpAcquisitionConfig::setSettlingMicros(uint16_t value) {
  _settlingMicros = value;
  return *this;
}

PwpAcquisitionConfig& PwpAcquisitionConfig::setDummyReads(uint8_t value) {
  _dummyReads = pwpClampU8(value, 0, 8);
  return *this;
}

PwpAcquisitionConfig& PwpAcquisitionConfig::setBurstSamples(uint8_t value) {
  _burstSamples = pwpClampU8(value, 1, PWP_MAX_BURST_SAMPLES);
  return *this;
}

PwpAcquisitionConfig& PwpAcquisitionConfig::enableSynchronousSampling(bool enabled) {
  _sync = enabled;
  return *this;
}

PwpAcquisitionConfig& PwpAcquisitionConfig::setInterSampleDelayMicros(uint16_t value) {
  _interSampleDelayMicros = value;
  return *this;
}

PwpAcquisitionConfig& PwpAcquisitionConfig::enableTrimmedBurstMean(bool enabled) {
  _trimmedBurstMean = enabled && !PWP_TINY_MODE;
  return *this;
}

uint16_t PwpAcquisitionConfig::settlingMicros() const { return _settlingMicros; }
uint8_t PwpAcquisitionConfig::dummyReads() const { return _dummyReads; }
uint8_t PwpAcquisitionConfig::burstSamples() const { return _burstSamples; }
bool PwpAcquisitionConfig::synchronousSampling() const { return _sync; }
uint16_t PwpAcquisitionConfig::interSampleDelayMicros() const { return _interSampleDelayMicros; }
bool PwpAcquisitionConfig::trimmedBurstMean() const { return _trimmedBurstMean; }

PwpFilterConfig::PwpFilterConfig()
  : _preset(PwpFilterPreset::STABLE), _emaAlpha(30), _movingAverageWindow(4),
    _medianEnabled(true), _enterWet(0.62f), _exitWet(0.48f), _outlierRejector(false),
    _outlierLimit(180), _hampelEnabled(false), _hampelThreshold(220) {}

PwpFilterConfig& PwpFilterConfig::setPreset(PwpFilterPreset preset) {
  _preset = preset;
  switch (preset) {
    case PwpFilterPreset::RAW:
    case PwpFilterPreset::DIAGNOSTIC_RAW:
      _emaAlpha = 100;
      _movingAverageWindow = 1;
      _medianEnabled = false;
      _outlierRejector = false;
      _hampelEnabled = false;
      _enterWet = 0.55f;
      _exitWet = 0.45f;
      break;
    case PwpFilterPreset::RESPONSIVE:
      _emaAlpha = 55;
      _movingAverageWindow = 2;
      _medianEnabled = true;
      _outlierRejector = false;
      _hampelEnabled = false;
      _enterWet = 0.58f;
      _exitWet = 0.46f;
      break;
    case PwpFilterPreset::STABLE:
      _emaAlpha = 30;
      _movingAverageWindow = 4;
      _medianEnabled = true;
      _outlierRejector = false;
      _hampelEnabled = false;
      _enterWet = 0.62f;
      _exitWet = 0.48f;
      break;
    case PwpFilterPreset::OUTLIER_REJECTING:
      _emaAlpha = 24;
      _movingAverageWindow = PWP_TINY_MODE ? 4 : 6;
      _medianEnabled = true;
      _outlierRejector = !PWP_TINY_MODE;
      _outlierLimit = 160;
      _hampelEnabled = (PWP_ENABLE_HAMPEL_FILTER != 0);
      _enterWet = 0.64f;
      _exitWet = 0.49f;
      break;
    case PwpFilterPreset::INDUSTRIAL_STABLE:
    default:
      _emaAlpha = 20;
      _movingAverageWindow = PWP_TINY_MODE ? 4 : 6;
      _medianEnabled = true;
      _outlierRejector = !PWP_TINY_MODE;
      _outlierLimit = 140;
      _hampelEnabled = (PWP_ENABLE_HAMPEL_FILTER != 0);
      _enterWet = 0.65f;
      _exitWet = 0.50f;
      break;
  }
  if (_movingAverageWindow > PWP_MOVING_AVG_WINDOW) _movingAverageWindow = PWP_MOVING_AVG_WINDOW;
  return *this;
}

PwpFilterConfig& PwpFilterConfig::setEmaAlpha(uint8_t alphaPercent) {
  _emaAlpha = pwpClampU8(alphaPercent, 1, 100);
  return *this;
}

PwpFilterConfig& PwpFilterConfig::setMovingAverageWindow(uint8_t samples) {
  _movingAverageWindow = pwpClampU8(samples, 1, PWP_MOVING_AVG_WINDOW);
  return *this;
}

PwpFilterConfig& PwpFilterConfig::setMedianEnabled(bool enabled) {
  _medianEnabled = enabled;
  return *this;
}

PwpFilterConfig& PwpFilterConfig::setHysteresis(float enterWet, float exitWet) {
  _enterWet = enterWet;
  _exitWet = exitWet;
  if (_exitWet > _enterWet) {
    float tmp = _exitWet;
    _exitWet = _enterWet;
    _enterWet = tmp;
  }
  return *this;
}

PwpFilterConfig& PwpFilterConfig::enableOutlierRejector(bool enabled) {
  _outlierRejector = enabled && !PWP_TINY_MODE;
  return *this;
}

PwpFilterConfig& PwpFilterConfig::setOutlierLimit(uint16_t maxStep) {
  _outlierLimit = maxStep;
  return *this;
}

PwpFilterConfig& PwpFilterConfig::enableHampelFilter(bool enabled) {
  _hampelEnabled = enabled && (PWP_ENABLE_HAMPEL_FILTER != 0);
  return *this;
}

PwpFilterConfig& PwpFilterConfig::setHampelThreshold(uint16_t threshold) {
  _hampelThreshold = threshold;
  return *this;
}

PwpFilterPreset PwpFilterConfig::preset() const { return _preset; }
uint8_t PwpFilterConfig::emaAlpha() const { return _emaAlpha; }
uint8_t PwpFilterConfig::movingAverageWindow() const { return _movingAverageWindow; }
bool PwpFilterConfig::medianEnabled() const { return _medianEnabled; }
float PwpFilterConfig::enterWetThreshold() const { return _enterWet; }
float PwpFilterConfig::exitWetThreshold() const { return _exitWet; }
bool PwpFilterConfig::outlierRejectorEnabled() const { return _outlierRejector; }
uint16_t PwpFilterConfig::outlierLimit() const { return _outlierLimit; }
bool PwpFilterConfig::hampelFilterEnabled() const { return _hampelEnabled; }
uint16_t PwpFilterConfig::hampelThreshold() const { return _hampelThreshold; }

PwpCalibrationConfig::PwpCalibrationConfig()
  : _calibrated(false), _dryRaw(0), _wetRaw(1023), _pointCount(0),
    _referenceTemperatureC(25.0f), _currentTemperatureC(25.0f), _temperatureCoefficient(0.0f) {
  for (uint8_t i = 0; i < PWP_MAX_CAL_POINTS; ++i) {
    _points[i].raw = 0;
    _points[i].wetness = 0.0f;
    _points[i].temperatureC = 25.0f;
  }
}

PwpCalibrationConfig& PwpCalibrationConfig::setDryWet(int dryRaw, int wetRaw) {
  _dryRaw = dryRaw;
  _wetRaw = wetRaw;
  _calibrated = (_dryRaw != _wetRaw);
  _pointCount = 0;
  addPoint(_dryRaw, 0.0f, _referenceTemperatureC);
  addPoint(_wetRaw, 1.0f, _referenceTemperatureC);
  return *this;
}

PwpCalibrationConfig& PwpCalibrationConfig::clear() {
  _calibrated = false;
  _dryRaw = 0;
  _wetRaw = 1023;
  _pointCount = 0;
  _referenceTemperatureC = 25.0f;
  _currentTemperatureC = 25.0f;
  _temperatureCoefficient = 0.0f;
  return *this;
}

PwpCalibrationConfig& PwpCalibrationConfig::clearPoints() {
  _pointCount = 0;
  _calibrated = false;
  return *this;
}

bool PwpCalibrationConfig::addPoint(int raw, float wetness) {
  return addPoint(raw, wetness, _referenceTemperatureC);
}

bool PwpCalibrationConfig::addPoint(int raw, float wetness, float temperatureC) {
  if (_pointCount >= PWP_MAX_CAL_POINTS) return false;
  _points[_pointCount].raw = raw;
  _points[_pointCount].wetness = clamp01(wetness);
  _points[_pointCount].temperatureC = temperatureC;
  _pointCount++;
  sortPoints();
  if (_pointCount >= 2) {
    _calibrated = true;
  }
  return true;
}

PwpCalibrationConfig& PwpCalibrationConfig::setReferenceTemperatureC(float value) {
  _referenceTemperatureC = value;
  return *this;
}

PwpCalibrationConfig& PwpCalibrationConfig::setTemperatureCoefficient(float wetnessPerC) {
  _temperatureCoefficient = wetnessPerC;
  return *this;
}

PwpCalibrationConfig& PwpCalibrationConfig::setCurrentTemperatureC(float value) {
  _currentTemperatureC = value;
  return *this;
}

bool PwpCalibrationConfig::calibrated() const { return _calibrated; }
int PwpCalibrationConfig::dryRaw() const { return _dryRaw; }
int PwpCalibrationConfig::wetRaw() const { return _wetRaw; }
uint8_t PwpCalibrationConfig::pointCount() const { return _pointCount; }
float PwpCalibrationConfig::referenceTemperatureC() const { return _referenceTemperatureC; }
float PwpCalibrationConfig::currentTemperatureC() const { return _currentTemperatureC; }
float PwpCalibrationConfig::temperatureCoefficient() const { return _temperatureCoefficient; }

PwpCalibrationPoint PwpCalibrationConfig::pointAt(uint8_t index) const {
  if (index >= _pointCount) {
    PwpCalibrationPoint empty = {0, 0.0f, _referenceTemperatureC};
    return empty;
  }
  return _points[index];
}

float PwpCalibrationConfig::normalize(int raw, uint8_t adcBits) const {
  float value = 0.0f;
  if (_calibrated && _pointCount >= 2) {
    if (raw <= _points[0].raw) {
      value = _points[0].wetness;
    } else if (raw >= _points[_pointCount - 1].raw) {
      value = _points[_pointCount - 1].wetness;
    } else {
      for (uint8_t i = 0; i < (uint8_t)(_pointCount - 1); ++i) {
        const PwpCalibrationPoint& a = _points[i];
        const PwpCalibrationPoint& b = _points[i + 1];
        if (raw >= a.raw && raw <= b.raw) {
          float span = (float)(b.raw - a.raw);
          float t = span == 0.0f ? 0.0f : ((float)raw - (float)a.raw) / span;
          value = a.wetness + t * (b.wetness - a.wetness);
          break;
        }
      }
    }
  } else if (_calibrated && _dryRaw != _wetRaw) {
    // span may be negative when dryRaw > wetRaw (inverted sensor wiring).
    // The division already maps dryRaw→0 and wetRaw→1 in both orientations;
    // no extra flip is needed or correct.
    float span = (float)(_wetRaw - _dryRaw);
    value = ((float)raw - (float)_dryRaw) / span;
  } else {
    long maxAdc = 1023;
    if (adcBits > 0 && adcBits < 31) maxAdc = (1L << adcBits) - 1L;
    if (maxAdc <= 0) maxAdc = 1023;
    value = (float)raw / (float)maxAdc;
  }

  if (_temperatureCoefficient != 0.0f) {
    value += (_currentTemperatureC - _referenceTemperatureC) * _temperatureCoefficient;
  }
  return clamp01(value);
}

PwpCalibrationProfile PwpCalibrationConfig::exportProfile() const {
  PwpCalibrationProfile profile;
  profile.magic = PWP_PROFILE_MAGIC;
  profile.version = (uint16_t)((PWP_VERSION_MAJOR << 8) | PWP_VERSION_MINOR);
  profile.pointCount = _pointCount;
  profile.dryRaw = _dryRaw;
  profile.wetRaw = _wetRaw;
  profile.referenceTemperatureC = _referenceTemperatureC;
  profile.temperatureCoefficientPerC = _temperatureCoefficient;
  for (uint8_t i = 0; i < PWP_MAX_CAL_POINTS; ++i) {
    profile.points[i] = _points[i];
  }
  profile.crc = computeProfileCrc(profile);
  return profile;
}

bool PwpCalibrationConfig::importProfile(const PwpCalibrationProfile& profile) {
  if (profile.magic != PWP_PROFILE_MAGIC) return false;
  if (profile.pointCount > PWP_MAX_CAL_POINTS) return false;
  if (computeProfileCrc(profile) != profile.crc) return false;
  _dryRaw = profile.dryRaw;
  _wetRaw = profile.wetRaw;
  _pointCount = profile.pointCount;
  _referenceTemperatureC = profile.referenceTemperatureC;
  // _currentTemperatureC is intentionally preserved so that any live
  // temperature the caller set via setCurrentTemperatureC() / setTemperatureC()
  // continues to apply after the profile is loaded.  Only the *reference*
  // temperature comes from the stored profile.
  _temperatureCoefficient = profile.temperatureCoefficientPerC;
  for (uint8_t i = 0; i < PWP_MAX_CAL_POINTS; ++i) {
    _points[i] = profile.points[i];
  }
  sortPoints();
  _calibrated = (_pointCount >= 2) || (_dryRaw != _wetRaw);
  return true;
}

uint16_t PwpCalibrationConfig::computeProfileCrc(const PwpCalibrationProfile& profile) {
  // CRC-16/CCITT-FALSE over a stable, quantized representation of the profile.
  // Polynomial: 0x1021, init: 0xFFFF. The stored crc field is intentionally
  // excluded so importProfile() rejects corrupted calibration profiles.
  uint16_t crc = 0xFFFFu;
  crc = pwpCrc16CcittUpdateU32(crc, profile.magic);
  crc = pwpCrc16CcittUpdateU16(crc, profile.version);
  crc = pwpCrc16CcittUpdate(crc, profile.pointCount);
  crc = pwpCrc16CcittUpdateI16(crc, profile.dryRaw);
  crc = pwpCrc16CcittUpdateI16(crc, profile.wetRaw);
  int refTemp100 = (int)(profile.referenceTemperatureC * 100.0f);
  int coeff10000 = (int)(profile.temperatureCoefficientPerC * 10000.0f);
  crc = pwpCrc16CcittUpdateI16(crc, refTemp100);
  crc = pwpCrc16CcittUpdateI16(crc, coeff10000);
  for (uint8_t i = 0; i < profile.pointCount && i < PWP_MAX_CAL_POINTS; ++i) {
    int wet1000 = (int)(profile.points[i].wetness * 1000.0f);
    int temp100 = (int)(profile.points[i].temperatureC * 100.0f);
    crc = pwpCrc16CcittUpdateI16(crc, profile.points[i].raw);
    crc = pwpCrc16CcittUpdateI16(crc, wet1000);
    crc = pwpCrc16CcittUpdateI16(crc, temp100);
  }
  return crc;
}

void PwpCalibrationConfig::sortPoints() {
  for (uint8_t i = 0; i < _pointCount; ++i) {
    for (uint8_t j = (uint8_t)(i + 1); j < _pointCount; ++j) {
      if (_points[j].raw < _points[i].raw) {
        PwpCalibrationPoint t = _points[i];
        _points[i] = _points[j];
        _points[j] = t;
      }
    }
  }
}

float PwpCalibrationConfig::clamp01(float value) const {
  if (value < 0.0f) return 0.0f;
  if (value > 1.0f) return 1.0f;
  return value;
}

PulseWetProbe::PulseWetProbe()
  : _begun(false), _touchActive(false), _reversibleMode(false),
    _sensePin(-1), _excitationPin(-1), _touchPin(-1),
    _adcBits(PWP_DEFAULT_ADC_BITS), _profile(PwpProfile::SOIL),
    _excitation(PwpExcitation::PULSED_HIGH_LOW), _acquisition(), _filters(), _calibration(),
    _capabilities(pwpDetectBoardCapabilities()), _baseIntervalMillis(PWP_DEFAULT_INTERVAL_MS),
    _sequence(0), _hasLastFiltered(false), _lastFiltered(0), _lastRaw(0), _lastWetnessPercent(0.0f),
    _chargeBalance(0), _state(PwpReadingState::UNKNOWN), _medianCount(0), _medianIndex(0),
    _movingCount(0), _movingIndex(0),
#if PWP_ENABLE_HAMPEL_FILTER
    _hampelCount(0), _hampelIndex(0),
#endif
    _hasConductivityTrendBaseline(false), _conductivityCompositeEma(0.0f),
    _conductivityLevelWeight(PWP_CONDUCTIVITY_LEVEL_WEIGHT_DEFAULT),
    _conductivityDiffWeight(PWP_CONDUCTIVITY_DIFF_WEIGHT_DEFAULT),
    _conductivityWeightsCustom(false),
    _stuckCount(0), _possibleOpenCount(0),
    _possibleShortCount(0), _lastBeginStatus(PwpBeginStatus::OK)
#if PWP_ENABLE_FAULT_LOG
    , _faultCount(0), _faultIndex(0)
#endif
{
  for (uint8_t i = 0; i < PWP_MEDIAN_WINDOW; ++i) _medianBuffer[i] = 0;
  for (uint8_t i = 0; i < PWP_MOVING_AVG_WINDOW; ++i) _movingBuffer[i] = 0;
#if PWP_ENABLE_HAMPEL_FILTER
  for (uint8_t i = 0; i < PWP_HAMPEL_WINDOW; ++i) _hampelBuffer[i] = 0;
#endif
#if PWP_ENABLE_FAULT_LOG
  for (uint8_t i = 0; i < PWP_FAULT_LOG_CAPACITY; ++i) {
    _faultLog[i].sequence = 0;
    _faultLog[i].timestampMillis = 0;
    _faultLog[i].flags = 0;
    _faultLog[i].noiseScore = 0;
    _faultLog[i].qualityScore = 0;
  }
#endif
}

bool PulseWetProbe::beginTwoPlate(int sensePin, int excitationPin) {
  return beginTwoPlate(sensePin, excitationPin, _capabilities.defaultAdcBits);
}

bool PulseWetProbe::beginTwoPlate(int sensePin, int excitationPin, uint8_t adcBits) {
  if (sensePin < 0 || excitationPin < 0 || sensePin == excitationPin) {
    _lastBeginStatus = PwpBeginStatus::INVALID_PIN;
    return false;
  }
  _sensePin = sensePin;
  _excitationPin = excitationPin;
  _touchPin = -1;
  _touchActive = false;
  _reversibleMode = false;
  _excitation = PwpExcitation::PULSED_HIGH_LOW;
  _adcBits = pwpClampU8(adcBits, PWP_MIN_ADC_BITS, PWP_MAX_ADC_BITS);
  configureSenseInput();
  restHiZ();

  applyAnalogResolution();

  _begun = true;
  _lastBeginStatus = PwpBeginStatus::OK;
  resetFilters();
  return true;
}

bool PulseWetProbe::beginReversibleTwoPlate(int plateAPin, int plateBPin) {
  return beginReversibleTwoPlate(plateAPin, plateBPin, _capabilities.defaultAdcBits);
}

bool PulseWetProbe::beginReversibleTwoPlate(int plateAPin, int plateBPin, uint8_t adcBits) {
  if (plateAPin < 0 || plateBPin < 0 || plateAPin == plateBPin) {
    _lastBeginStatus = PwpBeginStatus::INVALID_PIN;
    return false;
  }
  _sensePin = plateAPin;
  _excitationPin = plateBPin;
  _touchPin = -1;
  _touchActive = false;
  _reversibleMode = true;
  _excitation = PwpExcitation::REVERSIBLE_TWO_PLATE;
  _adcBits = pwpClampU8(adcBits, PWP_MIN_ADC_BITS, PWP_MAX_ADC_BITS);
  configureSenseInput();
  restHiZ();

  applyAnalogResolution();

  _begun = true;
  _lastBeginStatus = PwpBeginStatus::OK;
  resetFilters();
  return true;
}

bool PulseWetProbe::beginBalancedTwoPlate(int plateAPin, int plateBPin) {
  return beginReversibleTwoPlate(plateAPin, plateBPin);
}

bool PulseWetProbe::beginTouch(int touchPin) {
  if (touchPin < 0) {
    _lastBeginStatus = PwpBeginStatus::INVALID_PIN;
    return false;
  }
  _capabilities = pwpDetectBoardCapabilities();
  if (!_capabilities.hasNativeTouch || !supportsTouchRuntime()) {
    _touchActive = false;
    _lastBeginStatus = PwpBeginStatus::TOUCH_UNSUPPORTED_BOARD;
    return false;
  }
  if (!isTouchPinUsable(touchPin)) {
    _touchActive = false;
    _lastBeginStatus = PwpBeginStatus::TOUCH_UNSUPPORTED_PIN;
    return false;
  }
  _touchPin = touchPin;
  _sensePin = touchPin;
  _excitationPin = -1;
  _touchActive = true;
  _reversibleMode = false;
  _excitation = PwpExcitation::ESP32_TOUCH;
  _begun = true;
  _lastBeginStatus = PwpBeginStatus::OK;
  resetFilters();
  return true;
}

bool PulseWetProbe::beginSoil(int sensePin, int excitationPin) {
  enableBoardDefaults();
  setProfile(PwpProfile::SOIL);
  bool ok = beginTwoPlate(sensePin, excitationPin);
  setExcitation(PwpExcitation::PULSED_HIGH_LOW);
  filters().setPreset(PwpFilterPreset::STABLE);
  return ok;
}

bool PulseWetProbe::beginWetDry(int sensePin, int excitationPin) {
  enableBoardDefaults();
  setProfile(PwpProfile::WETNESS);
  bool ok = beginTwoPlate(sensePin, excitationPin);
  setExcitation(PwpExcitation::PULSED_HIGH_LOW);
  filters().setPreset(PwpFilterPreset::RESPONSIVE);
  return ok;
}

bool PulseWetProbe::beginConductivityTrend(int sensePin, int excitationPin) {
  enableBoardDefaults();
  setProfile(PwpProfile::CONDUCTIVITY_TREND);
  bool ok = beginTwoPlate(sensePin, excitationPin);
  setExcitation(PwpExcitation::PULSED_HIGH_LOW);
  filters().setPreset(PwpFilterPreset::INDUSTRIAL_STABLE);
  return ok;
}

bool PulseWetProbe::beginEsp32TouchWetness(int touchPin) {
  enableBoardDefaults();
  setProfile(PwpProfile::WETNESS);
  return beginTouch(touchPin);
}

PwpBeginStatus PulseWetProbe::lastBeginStatus() const { return _lastBeginStatus; }

PulseWetProbe& PulseWetProbe::setProfile(PwpProfile profile) {
  _profile = profile;
  return *this;
}

PulseWetProbe& PulseWetProbe::setExcitation(PwpExcitation excitation) {
  if (excitation == PwpExcitation::ESP32_TOUCH && !_touchActive) {
    _lastBeginStatus = PwpBeginStatus::TOUCH_UNSUPPORTED_BOARD;
    return *this;
  }
  _excitation = excitation;
  _reversibleMode = (excitation == PwpExcitation::REVERSIBLE_TWO_PLATE);
  return *this;
}

PulseWetProbe& PulseWetProbe::enableBoardDefaults() {
  _capabilities = pwpDetectBoardCapabilities();
  _adcBits = _capabilities.defaultAdcBits;
  _acquisition.setBurstSamples(_capabilities.recommendedBurstSamples);

  if (_capabilities.memoryTiny) {
    _filters.setPreset(PwpFilterPreset::STABLE);
    _acquisition.setDummyReads(1).setSettlingMicros(100).setInterSampleDelayMicros(PWP_INTER_SAMPLE_DELAY_TINY_US).enableTrimmedBurstMean(false);
  } else {
#if PWP_BOARD_ESP32
    _filters.setPreset(PwpFilterPreset::INDUSTRIAL_STABLE);
    _acquisition.setDummyReads(2).setSettlingMicros(80).setInterSampleDelayMicros(PWP_INTER_SAMPLE_DELAY_ESP32_US).enableTrimmedBurstMean(true);
#else
    if (_capabilities.supportsAdvancedFilters) {
      _filters.setPreset(PwpFilterPreset::OUTLIER_REJECTING);
      _acquisition.setDummyReads(2).setSettlingMicros(70).setInterSampleDelayMicros(PWP_INTER_SAMPLE_DELAY_FULL_US).enableTrimmedBurstMean(true);
    } else {
      _filters.setPreset(PwpFilterPreset::STABLE);
      _acquisition.setDummyReads(1).setSettlingMicros(90).setInterSampleDelayMicros(PWP_INTER_SAMPLE_DELAY_CORE_US).enableTrimmedBurstMean(false);
    }
#endif
  }

  applyAnalogResolution();
  return *this;
}

PulseWetProbe& PulseWetProbe::setBaseIntervalMillis(unsigned long value) {
  _baseIntervalMillis = value;
  return *this;
}

PulseWetProbe& PulseWetProbe::setAdcBits(uint8_t bits) {
  _adcBits = pwpClampU8(bits, PWP_MIN_ADC_BITS, PWP_MAX_ADC_BITS);
  applyAnalogResolution();
  return *this;
}

PulseWetProbe& PulseWetProbe::setTemperatureC(float value) {
  _calibration.setCurrentTemperatureC(value);
  return *this;
}

PulseWetProbe& PulseWetProbe::calibrateDryWet(int dryRaw, int wetRaw) {
  _calibration.setDryWet(dryRaw, wetRaw);
  return *this;
}

PulseWetProbe& PulseWetProbe::setConductivityTrendWeights(float levelWeight, float differentialWeight) {
  if (levelWeight < 0.0f) levelWeight = 0.0f;
  if (differentialWeight < 0.0f) differentialWeight = 0.0f;
  if ((levelWeight + differentialWeight) <= 0.0f) {
    levelWeight = PWP_CONDUCTIVITY_LEVEL_WEIGHT_DEFAULT;
    differentialWeight = PWP_CONDUCTIVITY_DIFF_WEIGHT_DEFAULT;
    _conductivityWeightsCustom = false;
  } else {
    _conductivityWeightsCustom = true;
  }
  _conductivityLevelWeight = levelWeight;
  _conductivityDiffWeight = differentialWeight;
  return *this;
}

PulseWetProbe& PulseWetProbe::resetConductivityTrendBaseline() {
  _hasConductivityTrendBaseline = false;
  _conductivityCompositeEma = 0.0f;
  return *this;
}

PwpAcquisitionConfig& PulseWetProbe::acquisition() { return _acquisition; }
PwpFilterConfig& PulseWetProbe::filters() { return _filters; }
PwpCalibrationConfig& PulseWetProbe::calibration() { return _calibration; }
const PwpAcquisitionConfig& PulseWetProbe::acquisition() const { return _acquisition; }
const PwpFilterConfig& PulseWetProbe::filters() const { return _filters; }
const PwpCalibrationConfig& PulseWetProbe::calibration() const { return _calibration; }

PwpReading PulseWetProbe::read() {
  PwpReading r;
  r.timestampMillis = millis();

  if (!_begun) {
    // Increment sequence so the caller can detect repeated failed calls, but
    // do NOT count these against the valid measurement stream.  Sequence for
    // real readings therefore starts at 1 only after a successful begin*().
    r.sequence = ++_sequence;
    r.diagnostics.needsCalibration = true;
    r.diagnostics.lowConfidence = true;
    r.qualityScore = 0;
    recordFaultIfNeeded(r);
    return r;
  }

  r.sequence = ++_sequence;

  PwpSampleResult forward = {0, 0};
  PwpSampleResult reverse = {0, 0};
  if (_touchActive || _excitation == PwpExcitation::ESP32_TOUCH) {
    forward = sampleTouchPad();
    reverse = forward;
    r.touchRaw = forward.value;
  } else if (_excitation == PwpExcitation::PULLUP_WETNESS) {
    forward = samplePullupWetness();
    reverse = forward;
  } else if (_reversibleMode || _excitation == PwpExcitation::REVERSIBLE_TWO_PLATE) {
    // True two-electrode role reversal: Plate B drives HIGH while Plate A is sensed,
    // then Plate A drives HIGH while Plate B is sensed. This is the balanced mode
    // intended for stronger anti-bias / anti-corrosion behaviour.
    forward = sampleBetweenPins(_excitationPin, _sensePin, HIGH);
    reverse = sampleBetweenPins(_sensePin, _excitationPin, HIGH);
  } else {
    // Backward-compatible high/low pulsed drive on a single excitation pin.
    // This is not true electrode role reversal.
    forward = sampleWithDrive(HIGH);
    if (_excitation == PwpExcitation::PULSED_HIGH_LOW) {
      reverse = sampleWithDrive(LOW);
    } else {
      reverse = forward;
    }
  }

  r.rawForward = forward.value;
  r.rawReverse = reverse.value;
  r.rawDifferential = pwpAbsInt(forward.value - reverse.value);
  r.rawAverage = (forward.value + reverse.value) / 2;
  r.burstSpread = (forward.spread > reverse.spread) ? forward.spread : reverse.spread;

  uint16_t noise = (uint16_t)pwpClampInt((int)r.burstSpread + (r.rawDifferential / 2), 0, 65535);
  int rawForFilter = 0;
  if (_touchActive || _excitation == PwpExcitation::ESP32_TOUCH) {
    rawForFilter = r.touchRaw;
  } else {
    // Wetness / moisture trend must be based on the absolute response level, not on
    // forward-vs-reverse asymmetry. rawDifferential is kept for diagnostics such as
    // polarization, corrosion-risk and wiring asymmetry detection.
    rawForFilter = r.rawAverage;
  }

  const bool hadPreviousFiltered = _hasLastFiltered;
  int previousFiltered = _lastFiltered;
  r.filtered = applyFilters(rawForFilter, noise);
  r.levelIndex = _calibration.normalize(r.filtered, _adcBits);
  r.normalizedWetness = r.levelIndex;
  {
    long maxAdc = maxAdcValue();
    r.differentialIndex = maxAdc <= 0 ? 0.0f : pwpClampFloat((float)r.rawDifferential / (float)maxAdc, 0.0f, 1.0f);
  }
  r.conductivityTrend = computeConductivityTrend(r.levelIndex, r.differentialIndex);
  _state = classifyState(r.levelIndex, noise);
  r.state = _state;
  _lastWetnessPercent = r.wetnessPercent();

  if (hadPreviousFiltered && pwpAbsInt(r.filtered - previousFiltered) <= 1) {
    if (_stuckCount < 255) _stuckCount++;
  } else {
    _stuckCount = 0;
  }

  r.diagnostics = diagnose(r, noise, previousFiltered, hadPreviousFiltered);
  r.qualityScore = computeQuality(r.diagnostics, noise);
  _lastRaw = rawForFilter;
  restHiZ();
  recordFaultIfNeeded(r);
  return r;
}

unsigned long PulseWetProbe::nextIntervalMillis() const {
  unsigned long interval = _baseIntervalMillis;
#if PWP_ENABLE_POWER_SCHEDULER
  if (_state == PwpReadingState::DRY) interval += (_baseIntervalMillis / 2);
  if (_state == PwpReadingState::DAMP || _state == PwpReadingState::WET ||
      _state == PwpReadingState::SATURATED) interval = _baseIntervalMillis;
  if (_chargeBalance > PWP_CHARGE_BALANCE_THRESHOLD) interval += (_baseIntervalMillis / 2);
  if (_stuckCount > PWP_STUCK_SCHEDULER_THRESHOLD) interval += (_baseIntervalMillis / 4);
#endif
  return interval;
}

PwpBoardCapabilities PulseWetProbe::capabilities() const { return _capabilities; }
uint32_t PulseWetProbe::sequence() const { return _sequence; }
int PulseWetProbe::lastFiltered() const { return _lastFiltered; }
PwpReadingState PulseWetProbe::state() const { return _state; }
bool PulseWetProbe::isBegun() const { return _begun; }
bool PulseWetProbe::touchActive() const { return _touchActive; }
float PulseWetProbe::wetnessPercent() const { return _lastWetnessPercent; }
const char* PulseWetProbe::stateName() const { return pwpReadingStateName(_state); }

void PulseWetProbe::resetFilters() {
  _hasLastFiltered = false;
  _lastFiltered = 0;
  _lastRaw = 0;
  _lastWetnessPercent = 0.0f;
  _chargeBalance = 0;
  _state = PwpReadingState::UNKNOWN;
  _medianCount = 0;
  _medianIndex = 0;
  _movingCount = 0;
  _movingIndex = 0;
#if PWP_ENABLE_HAMPEL_FILTER
  _hampelCount = 0;
  _hampelIndex = 0;
#endif
  _hasConductivityTrendBaseline = false;
  _conductivityCompositeEma = 0.0f;
  _stuckCount = 0;
  _possibleOpenCount = 0;
  _possibleShortCount = 0;
  for (uint8_t i = 0; i < PWP_MEDIAN_WINDOW; ++i) _medianBuffer[i] = 0;
  for (uint8_t i = 0; i < PWP_MOVING_AVG_WINDOW; ++i) _movingBuffer[i] = 0;
#if PWP_ENABLE_HAMPEL_FILTER
  for (uint8_t i = 0; i < PWP_HAMPEL_WINDOW; ++i) _hampelBuffer[i] = 0;
#endif
}

#if PWP_ENABLE_FAULT_LOG
uint8_t PulseWetProbe::faultLogCount() const { return _faultCount; }

PwpFaultEntry PulseWetProbe::faultAt(uint8_t index) const {
  if (index >= _faultCount || PWP_FAULT_LOG_CAPACITY == 0) {
    PwpFaultEntry empty = {0, 0, 0, 0, 0};
    return empty;
  }
  uint8_t start = (_faultCount == PWP_FAULT_LOG_CAPACITY) ? _faultIndex : 0;
  uint8_t realIndex = (uint8_t)((start + index) % PWP_FAULT_LOG_CAPACITY);
  return _faultLog[realIndex];
}

void PulseWetProbe::clearFaultLog() {
  _faultCount = 0;
  _faultIndex = 0;
}
#endif

PulseWetProbe::PwpSampleResult PulseWetProbe::sampleWithDrive(uint8_t driveLevel) {
  pinMode(_excitationPin, OUTPUT);
  digitalWrite(_excitationPin, driveLevel);
  delayMicroseconds(_acquisition.settlingMicros());
  PwpSampleResult result = acquireBurstAverage(_sensePin);
  if (driveLevel == HIGH) {
    _chargeBalance++;
  } else if (_chargeBalance > 0) {
    _chargeBalance--;
  }
  restHiZ();
  return result;
}

PulseWetProbe::PwpSampleResult PulseWetProbe::sampleBetweenPins(int drivePin, int sensePin, uint8_t driveLevel) {
  pinMode(sensePin, INPUT);
  pinMode(drivePin, OUTPUT);
  digitalWrite(drivePin, driveLevel);
  delayMicroseconds(_acquisition.settlingMicros());
  PwpSampleResult result = acquireBurstAverage(sensePin);
  // A reversible pair intentionally balances electrode roles over the two-step
  // acquisition cycle. Do not accumulate one-sided charge-balance debt here.
  restHiZ();
  return result;
}

PulseWetProbe::PwpSampleResult PulseWetProbe::samplePullupWetness() {
  pinMode(_excitationPin, INPUT_PULLUP);
  delayMicroseconds(_acquisition.settlingMicros());
  PwpSampleResult result = acquireBurstAverage(_sensePin);
  restHiZ();
  return result;
}

PulseWetProbe::PwpSampleResult PulseWetProbe::sampleTouchPad() {
  PwpSampleResult result;
  result.value = 0;
  result.spread = 0;
#if PWP_ENABLE_ESP32_TOUCH && PWP_COMPILE_ESP32_TOUCH
  uint8_t n = _acquisition.burstSamples();
  if (n == 0) n = 1;
  if (n > PWP_MAX_BURST_SAMPLES) n = PWP_MAX_BURST_SAMPLES;
  uint32_t sum = 0;
  uint32_t low = 0xFFFFFFFFu;
  uint32_t high = 0;
  for (uint8_t i = 0; i < n; ++i) {
    uint32_t v = (uint32_t)touchRead((uint8_t)_touchPin);
    sum += v;
    if (v < low) low = v;
    if (v > high) high = v;
    if (_acquisition.interSampleDelayMicros() > 0) delayMicroseconds(_acquisition.interSampleDelayMicros());
  }
  result.value = (int)(sum / n);
  result.spread = (uint16_t)((high > low) ? (high - low > 65535u ? 65535u : high - low) : 0u);
#else
  result.value = 0;
  result.spread = 65535u;
#endif
  return result;
}

PulseWetProbe::PwpSampleResult PulseWetProbe::acquireBurstAverage(int sensePin) {
  PwpSampleResult result;
  result.value = 0;
  result.spread = 0;
  uint8_t dummy = _acquisition.dummyReads();
  for (uint8_t i = 0; i < dummy; ++i) {
    (void)analogRead(sensePin);
    if (_acquisition.interSampleDelayMicros() > 0) delayMicroseconds(_acquisition.interSampleDelayMicros());
  }

  uint8_t n = _acquisition.burstSamples();
  if (n == 0) n = 1;
  if (n > PWP_MAX_BURST_SAMPLES) n = PWP_MAX_BURST_SAMPLES;
  int minValue = 32767;
  int maxValue = -32768;
  long sum = 0;
  for (uint8_t i = 0; i < n; ++i) {
    int v = analogRead(sensePin);
    if (v < minValue) minValue = v;
    if (v > maxValue) maxValue = v;
    sum += v;
    if (_acquisition.synchronousSampling() && _acquisition.interSampleDelayMicros() > 0) {
      delayMicroseconds(_acquisition.interSampleDelayMicros());
    }
  }

  if (_acquisition.trimmedBurstMean() && n >= 5) {
    sum -= minValue;
    sum -= maxValue;
    result.value = (int)(sum / (n - 2));
  } else {
    result.value = (int)(sum / n);
  }
  result.spread = (uint16_t)pwpClampInt(maxValue - minValue, 0, 65535);
  return result;
}

int PulseWetProbe::applyFilters(int raw, uint16_t noise) {
  int value = raw;
#if PWP_ENABLE_HAMPEL_FILTER
  if (_filters.hampelFilterEnabled()) {
    value = applyHampel(value);
  }
#endif
  if (_filters.outlierRejectorEnabled()) {
    value = applyOutlierRejector(value, noise);
  }
  if (_filters.preset() != PwpFilterPreset::RAW &&
      _filters.preset() != PwpFilterPreset::DIAGNOSTIC_RAW &&
      _filters.medianEnabled()) {
    value = applyMedian(value);
  }
  if (_filters.preset() != PwpFilterPreset::RAW &&
      _filters.preset() != PwpFilterPreset::DIAGNOSTIC_RAW &&
      _filters.movingAverageWindow() > 1) {
    value = applyMovingAverage(value);
  }
  uint8_t alpha = _filters.emaAlpha();
  if (noise > PWP_FILTER_NOISE_ADAPT_THRESHOLD && alpha < 100) {
    uint8_t reduction = (uint8_t)pwpClampInt((int)noise / PWP_FILTER_EMA_NOISE_DIVISOR, 0, PWP_FILTER_EMA_MAX_REDUCTION);
    alpha = (alpha > reduction + PWP_FILTER_MIN_EMA_ALPHA) ? (uint8_t)(alpha - reduction) : PWP_FILTER_MIN_EMA_ALPHA;
  }
  if (!_hasLastFiltered || alpha >= 100) {
    _lastFiltered = value;
    _hasLastFiltered = true;
  } else {
    _lastFiltered = (int)(((long)alpha * value + (long)(100 - alpha) * _lastFiltered) / 100L);
  }
  return _lastFiltered;
}

int PulseWetProbe::applyOutlierRejector(int raw, uint16_t noise) const {
  if (!_hasLastFiltered) return raw;
  int delta = raw - _lastFiltered;
  int limit = (int)_filters.outlierLimit();
  if (noise > PWP_FILTER_NOISE_ADAPT_THRESHOLD) {
    int tighten = pwpClampInt((int)noise / PWP_OUTLIER_NOISE_DIVISOR, 0, limit / 2);
    limit -= tighten;
    if (limit < PWP_OUTLIER_MIN_LIMIT) limit = PWP_OUTLIER_MIN_LIMIT;
  }
  if (delta > limit) return _lastFiltered + limit;
  if (delta < -limit) return _lastFiltered - limit;
  return raw;
}

int PulseWetProbe::applyMedian(int raw) {
  _medianBuffer[_medianIndex] = raw;
  _medianIndex = (uint8_t)((_medianIndex + 1) % PWP_MEDIAN_WINDOW);
  if (_medianCount < PWP_MEDIAN_WINDOW) _medianCount++;

  int tmp[PWP_MEDIAN_WINDOW];
  for (uint8_t i = 0; i < _medianCount; ++i) tmp[i] = _medianBuffer[i];
  for (uint8_t i = 0; i < _medianCount; ++i) {
    for (uint8_t j = (uint8_t)(i + 1); j < _medianCount; ++j) {
      if (tmp[j] < tmp[i]) {
        int t = tmp[i];
        tmp[i] = tmp[j];
        tmp[j] = t;
      }
    }
  }
  return tmp[_medianCount / 2];
}

int PulseWetProbe::applyMovingAverage(int raw) {
  _movingBuffer[_movingIndex] = raw;
  _movingIndex = (uint8_t)((_movingIndex + 1) % PWP_MOVING_AVG_WINDOW);
  // Clamp first so a window reduction (e.g. via setPreset) takes effect
  // immediately without requiring a resetFilters() call.
  if (_movingCount > _filters.movingAverageWindow()) {
    _movingCount = _filters.movingAverageWindow();
  }
  if (_movingCount < _filters.movingAverageWindow()) _movingCount++;

  // Read the most-recent _movingCount samples in circular order.
  // _movingIndex now points to the slot AFTER the newest write, so the
  // oldest of the kept samples lives at (_movingIndex + PWP_MOVING_AVG_WINDOW
  // - _movingCount) % PWP_MOVING_AVG_WINDOW.
  long sum = 0;
  uint8_t n = _movingCount;
  uint8_t start = (uint8_t)((_movingIndex + PWP_MOVING_AVG_WINDOW - n) % PWP_MOVING_AVG_WINDOW);
  for (uint8_t i = 0; i < n; ++i)
    sum += _movingBuffer[(uint8_t)((start + i) % PWP_MOVING_AVG_WINDOW)];
  return (int)(sum / (n == 0 ? 1 : n));
}

#if PWP_ENABLE_HAMPEL_FILTER
int PulseWetProbe::applyHampel(int raw) {
  if (_hampelCount < 3) {
    _hampelBuffer[_hampelIndex] = raw;
    _hampelIndex = (uint8_t)((_hampelIndex + 1) % PWP_HAMPEL_WINDOW);
    if (_hampelCount < PWP_HAMPEL_WINDOW) _hampelCount++;
    return raw;
  }

  int tmp[PWP_HAMPEL_WINDOW];
  for (uint8_t i = 0; i < _hampelCount; ++i) tmp[i] = _hampelBuffer[i];
  for (uint8_t i = 0; i < _hampelCount; ++i) {
    for (uint8_t j = (uint8_t)(i + 1); j < _hampelCount; ++j) {
      if (tmp[j] < tmp[i]) {
        int t = tmp[i];
        tmp[i] = tmp[j];
        tmp[j] = t;
      }
    }
  }
  int median = tmp[_hampelCount / 2];
  int accepted = raw;
  if (pwpAbsInt(raw - median) > (int)_filters.hampelThreshold()) {
    accepted = median;
  }
  _hampelBuffer[_hampelIndex] = accepted;
  _hampelIndex = (uint8_t)((_hampelIndex + 1) % PWP_HAMPEL_WINDOW);
  if (_hampelCount < PWP_HAMPEL_WINDOW) _hampelCount++;
  return accepted;
}
#endif

PwpDiagnostics PulseWetProbe::diagnose(const PwpReading& reading, uint16_t noise, int previousFiltered, bool hadPreviousFiltered) {
  PwpDiagnostics d;
  d.noiseScore = noise;
  d.stabilityScore = (noise >= 1000) ? 0 : (uint16_t)(100 - pwpClampInt((int)noise / 10, 0, 100));
  d.needsCalibration = !_calibration.calibrated();
  d.unstable = noise > (PWP_TINY_MODE ? PWP_DIAGNOSTIC_NOISE_UNSTABLE_TINY : PWP_DIAGNOSTIC_NOISE_UNSTABLE_CORE);
  d.lowConfidence = d.needsCalibration || d.unstable;
  d.driftScore = hadPreviousFiltered ? (uint16_t)pwpClampInt(pwpAbsInt(reading.filtered - previousFiltered), 0, 1000) : 0;
  d.driftSuspected = hadPreviousFiltered && d.driftScore > (uint16_t)((uint32_t)_filters.outlierLimit() * 2u);
  d.stuckReading = _stuckCount >= PWP_STUCK_READING_THRESHOLD;
  d.cableNoiseSuspected = reading.burstSpread > (PWP_TINY_MODE ? PWP_CABLE_NOISE_TINY : PWP_CABLE_NOISE_CORE);

  long maxAdc = maxAdcValue();
  d.saturationLow = (reading.rawForward <= 2 && reading.rawReverse <= 2);
  d.saturationHigh = (reading.rawForward >= (int)maxAdc - 2 && reading.rawReverse >= (int)maxAdc - 2);
  d.possibleShort = d.saturationLow || (reading.rawDifferential < 2 && reading.rawAverage < 8);
  d.possibleOpen = d.saturationHigh || (reading.rawDifferential < 2 && reading.rawAverage > (int)maxAdc - 8);
  if (d.possibleShort) {
    if (_possibleShortCount < 255) _possibleShortCount++;
  } else {
    _possibleShortCount = 0;
  }
  if (d.possibleOpen) {
    if (_possibleOpenCount < 255) _possibleOpenCount++;
  } else {
    _possibleOpenCount = 0;
  }
  d.shortCircuit = d.possibleShort && _possibleShortCount >= PWP_FAULT_CONFIRM_WINDOWS;
  d.openCircuit = d.possibleOpen && _possibleOpenCount >= PWP_FAULT_CONFIRM_WINDOWS;
  {
    int asymmetryLimit = (int)(maxAdc / PWP_POLARIZATION_ASYMMETRY_DIVISOR);
    if (asymmetryLimit < PWP_POLARIZATION_ASYMMETRY_MIN) asymmetryLimit = PWP_POLARIZATION_ASYMMETRY_MIN;
    const bool usefulMidRange = reading.rawAverage > 4 && reading.rawAverage < (int)maxAdc - 4;
    const bool polarizationAsymmetry =
        (_excitation == PwpExcitation::PULSED_HIGH_LOW) &&
        usefulMidRange &&
        (reading.rawDifferential > asymmetryLimit);
    d.corrosionRisk = (_chargeBalance > PWP_CHARGE_BALANCE_THRESHOLD) || polarizationAsymmetry;
  }
  d.foulingScore = (uint16_t)pwpClampInt(
      (int)(reading.differentialIndex * PWP_FOULING_WEIGHT_POLARITY_IMBALANCE) +
      (d.stuckReading ? PWP_FOULING_WEIGHT_STUCK_READING : 0) +
      (d.driftSuspected ? PWP_FOULING_WEIGHT_DRIFT : 0) +
      (d.cableNoiseSuspected ? PWP_FOULING_WEIGHT_CABLE_NOISE : 0) +
      (d.unstable ? PWP_FOULING_WEIGHT_UNSTABLE : 0) +
      (d.corrosionRisk ? PWP_FOULING_WEIGHT_CORROSION_RISK : 0) +
      ((d.possibleOpen || d.possibleShort) ? PWP_FOULING_WEIGHT_WIRING_SUSPECT : 0),
      0, 100);
  d.touchUnsupported = (_excitation == PwpExcitation::ESP32_TOUCH && !_touchActive);
  if (d.possibleOpen || d.possibleShort || d.cableNoiseSuspected || d.driftSuspected || d.stuckReading) d.lowConfidence = true;
  return d;
}

PwpReadingState PulseWetProbe::classifyState(float levelIndex, uint16_t noise) const {
  if (!_calibration.calibrated()) return PwpReadingState::NEEDS_CALIBRATION;
  if (noise > (PWP_TINY_MODE ? PWP_NOISE_UNSTABLE_TINY : PWP_NOISE_UNSTABLE_CORE)) return PwpReadingState::UNSTABLE;
  if (levelIndex < PWP_THRESHOLD_DRY) return PwpReadingState::DRY;
  if (levelIndex < PWP_THRESHOLD_DAMP) return PwpReadingState::DAMP;
  if (levelIndex < PWP_THRESHOLD_WET) return PwpReadingState::WET;
  return PwpReadingState::SATURATED;
}

float PulseWetProbe::computeConductivityTrend(float levelIndex, float differentialIndex) {
  float levelWeight = _conductivityWeightsCustom ? _conductivityLevelWeight : PWP_CONDUCTIVITY_LEVEL_WEIGHT_DEFAULT;
  float diffWeight = _conductivityWeightsCustom ? _conductivityDiffWeight : PWP_CONDUCTIVITY_DIFF_WEIGHT_DEFAULT;
  if (!_conductivityWeightsCustom) {
    // Priority (highest to lowest): profile-specific > reversible-mode > default.
    // Check profile first so that CONDUCTIVITY_TREND / SURFACE_BRINE_TREND weights
    // are not silently overridden when reversibleMode is also active.
    if (_profile == PwpProfile::CONDUCTIVITY_TREND || _profile == PwpProfile::SURFACE_BRINE_TREND) {
      levelWeight = PWP_CONDUCTIVITY_LEVEL_WEIGHT_CONDUCTIVITY_PROFILE;
      diffWeight  = PWP_CONDUCTIVITY_DIFF_WEIGHT_CONDUCTIVITY_PROFILE;
    } else if (_reversibleMode || _excitation == PwpExcitation::REVERSIBLE_TWO_PLATE) {
      levelWeight = PWP_CONDUCTIVITY_LEVEL_WEIGHT_REVERSIBLE;
      diffWeight  = PWP_CONDUCTIVITY_DIFF_WEIGHT_REVERSIBLE;
    }
  }
  const float sum = levelWeight + diffWeight;
  if (sum <= 0.0f) {
    levelWeight = PWP_CONDUCTIVITY_LEVEL_WEIGHT_DEFAULT;
    diffWeight = PWP_CONDUCTIVITY_DIFF_WEIGHT_DEFAULT;
  } else {
    levelWeight /= sum;
    diffWeight /= sum;
  }
  float composite = pwpClampFloat((levelIndex * levelWeight) + (differentialIndex * diffWeight), 0.0f, 1.0f);
  if (!_hasConductivityTrendBaseline) {
    _conductivityCompositeEma = composite;
    _hasConductivityTrendBaseline = true;
    return 0.0f;
  }
  float delta = composite - _conductivityCompositeEma;
  if (delta < 0.0f) delta = -delta;
  _conductivityCompositeEma = (_conductivityCompositeEma * (1.0f - PWP_CONDUCTIVITY_TREND_EMA_ALPHA)) +
                              (composite * PWP_CONDUCTIVITY_TREND_EMA_ALPHA);
  // Trend score: 0 means stable/no observed change, 1 means strong relative movement.
  // This is intentionally not an EC/salinity measurement and is not equal to wetness.
  return pwpClampFloat(delta * PWP_CONDUCTIVITY_TREND_GAIN, 0.0f, 1.0f);
}

uint8_t PulseWetProbe::computeQuality(const PwpDiagnostics& diagnostics, uint16_t noise) const {
  int q = 100;
  if (diagnostics.needsCalibration) q -= PWP_QUALITY_PENALTY_NEEDS_CALIBRATION;
  if (diagnostics.unstable) q -= PWP_QUALITY_PENALTY_UNSTABLE;
  if (diagnostics.lowConfidence) q -= PWP_QUALITY_PENALTY_LOW_CONFIDENCE;
  if (diagnostics.openCircuit || diagnostics.shortCircuit) q -= PWP_QUALITY_PENALTY_SHORT_OPEN;
  if (diagnostics.possibleOpen || diagnostics.possibleShort) q -= PWP_QUALITY_PENALTY_POSSIBLE_FAULT;
  if (diagnostics.corrosionRisk) q -= PWP_QUALITY_PENALTY_CORROSION;
  if (diagnostics.cableNoiseSuspected) q -= PWP_QUALITY_PENALTY_CABLE_NOISE;
  if (diagnostics.stuckReading) q -= PWP_QUALITY_PENALTY_STUCK;
  if (diagnostics.driftSuspected) q -= PWP_QUALITY_PENALTY_DRIFT;
  if (diagnostics.touchUnsupported) q -= PWP_QUALITY_PENALTY_TOUCH_UNSUPPORTED;
  q -= pwpClampInt((int)noise / PWP_QUALITY_NOISE_DIVISOR, 0, PWP_QUALITY_NOISE_MAX_PENALTY);
  return (uint8_t)pwpClampInt(q, 0, 100);
}

void PulseWetProbe::recordFaultIfNeeded(const PwpReading& reading) {
#if PWP_ENABLE_FAULT_LOG
  if (reading.diagnostics.ok() || PWP_FAULT_LOG_CAPACITY == 0) return;
  uint8_t flags = 0;
  if (reading.diagnostics.openCircuit) flags |= 0x01;
  if (reading.diagnostics.shortCircuit) flags |= 0x02;
  if (reading.diagnostics.unstable) flags |= 0x04;
  if (reading.diagnostics.lowConfidence) flags |= 0x08;
  if (reading.diagnostics.corrosionRisk) flags |= 0x10;
  if (reading.diagnostics.cableNoiseSuspected) flags |= 0x20;
  if (reading.diagnostics.stuckReading) flags |= 0x40;
  if (reading.diagnostics.driftSuspected) flags |= 0x80;
  _faultLog[_faultIndex].sequence = reading.sequence;
  _faultLog[_faultIndex].timestampMillis = reading.timestampMillis;
  _faultLog[_faultIndex].flags = flags;
  _faultLog[_faultIndex].noiseScore = reading.diagnostics.noiseScore;
  _faultLog[_faultIndex].qualityScore = reading.qualityScore;
  _faultIndex = (uint8_t)((_faultIndex + 1) % PWP_FAULT_LOG_CAPACITY);
  if (_faultCount < PWP_FAULT_LOG_CAPACITY) _faultCount++;
#else
  (void)reading;
#endif
}

void PulseWetProbe::applyAnalogResolution() {
#if defined(ARDUINO) && (PWP_BOARD_SAMD || PWP_BOARD_RENESAS || PWP_BOARD_MBED_OR_NRF52 || PWP_BOARD_RP2040 || PWP_BOARD_STM32 || PWP_BOARD_TEENSY || PWP_BOARD_ESP32)
  analogReadResolution(_adcBits);
#endif
}

void PulseWetProbe::restHiZ() {
  if (_excitationPin >= 0) {
    pinMode(_excitationPin, INPUT);
  }
  if (_reversibleMode && _sensePin >= 0) {
    pinMode(_sensePin, INPUT);
  }
}

void PulseWetProbe::configureSenseInput() {
  if (_sensePin >= 0) {
    pinMode(_sensePin, INPUT);
  }
}

long PulseWetProbe::maxAdcValue() const {
  long maxAdc = 1023;
  if (_adcBits > 0 && _adcBits < 31) maxAdc = (1L << _adcBits) - 1L;
  if (maxAdc <= 0) maxAdc = 1023;
  return maxAdc;
}


bool PulseWetProbe::supportsTouchRuntime() const {
#if PWP_ENABLE_ESP32_TOUCH && PWP_COMPILE_ESP32_TOUCH
  return true;
#else
  return false;
#endif
}

bool PulseWetProbe::isTouchPinUsable(int touchPin) const {
#if PWP_ENABLE_ESP32_TOUCH && PWP_COMPILE_ESP32_TOUCH
  if (touchPin < 0) return false;
  /* Best-effort ESP32 touch-pin validation.
   * Arduino-ESP32 validates pins internally via digitalPinToTouchChannel() and
   * returns 0 for invalid touch pins, but rejecting obviously unsupported pins
   * before beginTouch() avoids silent bad data. Keep this conservative because
   * ESP32 variants expose different touch GPIO maps.
   */
  #if defined(ARDUINO) && defined(ARDUINO_ARCH_ESP32) && defined(digitalPinToTouchChannel)
    return digitalPinToTouchChannel((uint8_t)touchPin) >= 0;
  #elif defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3)
    return touchPin >= 1 && touchPin <= 14;
  #elif defined(CONFIG_IDF_TARGET_ESP32)
    switch (touchPin) {
      case 0: case 2: case 4: case 12: case 13: case 14: case 15: case 27: case 32: case 33:
        return true;
      default:
        return false;
    }
  #else
    /* Unknown ESP32-family target with SOC_TOUCH_SENSOR_SUPPORTED. Do not guess
     * a full pin map; allow the official core to perform its internal check and
     * make the documentation require a touch-capable GPIO.
     */
    return true;
  #endif
#else
  (void)touchPin;
  return false;
#endif
}
