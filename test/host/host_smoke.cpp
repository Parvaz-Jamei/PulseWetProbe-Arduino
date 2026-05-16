#include <PulseWetProbe.h>
#include <cstdio>
#include <cstring>

extern "C" void pwpHostSetAnalogSequence(const int* values, size_t count);
extern "C" void pwpHostClearAnalogSequence();

static int fail(int code) {
  std::printf("FAIL %d\n", code);
  return code;
}

int main() {
  PulseWetProbe probe;
  probe.enableBoardDefaults();
  if (!probe.beginTwoPlate(0, 7)) return fail(2);
  probe.setProfile(PwpProfile::SOIL);
  probe.filters().setPreset(PwpFilterPreset::INDUSTRIAL_STABLE);
  probe.calibration().setDryWet(120, 850);
#if PWP_MAX_CAL_POINTS > 2
  if (!probe.calibration().addPoint(512, 0.50f)) return fail(3);
#endif
  probe.setTemperatureC(25.0f);

  PwpCalibrationProfile profile = probe.calibration().exportProfile();
  PwpCalibrationConfig imported;
  if (!imported.importProfile(profile)) return fail(4);
  PwpCalibrationProfile corrupted = profile;
  corrupted.wetRaw += 1;
  if (imported.importProfile(corrupted)) return fail(16);

  PwpReading r = probe.read();
  if (r.diagnostics.driftSuspected) return fail(9);
  if (r.levelIndex <= 0.0f || r.wetnessPercent() <= 0.0f) return fail(10);
  if (std::strcmp(r.stateName(), "damp") != 0 &&
      std::strcmp(r.stateName(), "wet") != 0 &&
      std::strcmp(r.stateName(), "saturated") != 0) return fail(11);
  char line[PWP_CSV_BUFFER_SIZE];
  r.toCsv(line, sizeof(line));
  std::puts(line);

  PulseWetProbe polarityProbe;
  polarityProbe.enableBoardDefaults();
  if (!polarityProbe.beginTwoPlate(0, 7)) return fail(7);
  polarityProbe.acquisition().setDummyReads(0).setBurstSamples(1);
  polarityProbe.filters().setPreset(PwpFilterPreset::RAW);
  polarityProbe.calibration().setDryWet(0, 1023);
  const int alternatingSamples[] = {800, 600};
  pwpHostSetAnalogSequence(alternatingSamples, sizeof(alternatingSamples) / sizeof(alternatingSamples[0]));
  PwpReading pr = polarityProbe.read();
  pwpHostClearAnalogSequence();
  if (pr.rawAverage != 700 || pr.rawDifferential != 200 || pr.filtered != pr.rawAverage) {
    return fail(8);
  }


  PulseWetProbe highOnlyProbe;
  highOnlyProbe.enableBoardDefaults();
  if (!highOnlyProbe.beginTwoPlate(0, 7, 10)) return fail(23);
  highOnlyProbe.setExcitation(PwpExcitation::PULSED_HIGH_ONLY);
  highOnlyProbe.acquisition().setDummyReads(0).setBurstSamples(1);
  highOnlyProbe.filters().setPreset(PwpFilterPreset::RAW);
  highOnlyProbe.calibrateDryWet(0, 1023);
  const int highOnlySamples[] = {800, 600};
  pwpHostSetAnalogSequence(highOnlySamples, sizeof(highOnlySamples) / sizeof(highOnlySamples[0]));
  PwpReading ho0 = highOnlyProbe.read();
  PwpReading ho1 = highOnlyProbe.read();
  pwpHostClearAnalogSequence();
  if (ho0.rawForward != 800 || ho0.rawReverse != 800 || ho0.rawDifferential != 0) return fail(24);
  if (ho1.rawForward != 600 || ho1.rawReverse != 600 || ho1.rawDifferential != 0) return fail(25);

  PulseWetProbe reversibleProbe;
  reversibleProbe.enableBoardDefaults();
  if (!reversibleProbe.beginReversibleTwoPlate(0, 1, 10)) return fail(12);
  reversibleProbe.acquisition().setDummyReads(0).setBurstSamples(1);
  reversibleProbe.filters().setPreset(PwpFilterPreset::RAW);
  reversibleProbe.calibrateDryWet(0, 1023);
  const int reversibleSamples[] = {900, 100};
  pwpHostSetAnalogSequence(reversibleSamples, sizeof(reversibleSamples) / sizeof(reversibleSamples[0]));
  PwpReading rr = reversibleProbe.read();
  pwpHostClearAnalogSequence();
  if (rr.rawForward != 900 || rr.rawReverse != 100 || rr.differentialIndex <= 0.70f) {
    return fail(13);
  }

  PulseWetProbe easyProbe;
  if (!easyProbe.beginSoil(0, 7)) return fail(14);
  easyProbe.calibrateDryWet(0, 1023);
  PwpReading er = easyProbe.read();
  if (er.wetnessPercent() < 0.0f || er.wetnessPercent() > 100.0f) return fail(15);

#if PWP_ENABLE_HAMPEL_FILTER
  PulseWetProbe hampelProbe;
  hampelProbe.enableBoardDefaults();
  if (!hampelProbe.beginTwoPlate(0, 7, 10)) return fail(17);
  hampelProbe.acquisition().setDummyReads(0).setBurstSamples(1);
  hampelProbe.filters().setPreset(PwpFilterPreset::RAW).enableHampelFilter(true).setHampelThreshold(120).setEmaAlpha(100);
  hampelProbe.calibrateDryWet(0, 1023);
  const int hampelSamples[] = {500,500, 500,500, 500,500, 500,500, 1023,1023};
  pwpHostSetAnalogSequence(hampelSamples, sizeof(hampelSamples) / sizeof(hampelSamples[0]));
  for (int i = 0; i < 4; ++i) (void)hampelProbe.read();
  PwpReading spike = hampelProbe.read();
  pwpHostClearAnalogSequence();
  if (spike.filtered > 650) return fail(18);
#endif

  PulseWetProbe trendProbe;
  trendProbe.enableBoardDefaults();
  if (!trendProbe.beginReversibleTwoPlate(0, 1, 10)) return fail(19);
  trendProbe.acquisition().setDummyReads(0).setBurstSamples(1);
  trendProbe.filters().setPreset(PwpFilterPreset::RAW).setEmaAlpha(100);
  trendProbe.setProfile(PwpProfile::CONDUCTIVITY_TREND);
  trendProbe.calibrateDryWet(0, 1023);
  const int trendSamples[] = {500,500, 100,900};
  pwpHostSetAnalogSequence(trendSamples, sizeof(trendSamples) / sizeof(trendSamples[0]));
  PwpReading t0 = trendProbe.read();
  PwpReading t1 = trendProbe.read();
  pwpHostClearAnalogSequence();
  if (t0.conductivityTrend != 0.0f) return fail(20);
  if (t1.conductivityTrend <= 0.20f) return fail(21);
  if ((int)(t1.conductivityTrend * 1000.0f) == (int)(t1.normalizedWetness * 1000.0f)) return fail(22);

  // Regression test: moving average must track new values after buffer wraps.
  // Bug: _movingCount was bounded by movingAverageWindow() while _movingIndex
  // walked the full PWP_MOVING_AVG_WINDOW, so new samples beyond the initial
  // fill window were written outside the read range and silently dropped.
  {
    PulseWetProbe maProbe;
    maProbe.enableBoardDefaults();
    if (!maProbe.beginTwoPlate(0, 7, 10)) return fail(26);
    maProbe.acquisition().setDummyReads(0).setBurstSamples(1);
    maProbe.filters()
        .setPreset(PwpFilterPreset::RAW)
        .setMovingAverageWindow(4)
        .setMedianEnabled(false)
        .setEmaAlpha(100);
    maProbe.calibrateDryWet(0, 1023);
    // 8 high samples fill and wrap the buffer, then 4 zero samples
    const int maSamples[] = {800,800,800,800,800,800,800,800,0,0,0,0};
    pwpHostSetAnalogSequence(maSamples, sizeof(maSamples) / sizeof(maSamples[0]));
    for (int i = 0; i < 11; ++i) (void)maProbe.read();
    PwpReading maLast = maProbe.read();
    pwpHostClearAnalogSequence();
    // After 4 consecutive zero samples the 4-sample window must average to 0
    if (maLast.filtered != 0) return fail(27);
  }

  // Regression test: CONDUCTIVITY_TREND profile weights must not be overridden
  // by reversible-mode weights when both are active simultaneously.
  {
    PulseWetProbe wProbe;
    wProbe.enableBoardDefaults();
    if (!wProbe.beginReversibleTwoPlate(0, 1, 10)) return fail(28);
    wProbe.acquisition().setDummyReads(0).setBurstSamples(1);
    wProbe.filters().setPreset(PwpFilterPreset::RAW).setEmaAlpha(100);
    wProbe.setProfile(PwpProfile::CONDUCTIVITY_TREND);
    wProbe.calibrateDryWet(0, 1023);
    const int wSamples[] = {500,500, 100,900};
    pwpHostSetAnalogSequence(wSamples, sizeof(wSamples) / sizeof(wSamples[0]));
    (void)wProbe.read();
    PwpReading w1 = wProbe.read();
    pwpHostClearAnalogSequence();
    // With CONDUCTIVITY_TREND profile the differential weight is heavier (0.55)
    // than with reversible-only (0.45), so the trend score must be > 0
    if (w1.conductivityTrend <= 0.0f) return fail(29);
  }

  // Regression test: _movingCount must shrink immediately when movingAverageWindow
  // is reduced (e.g. via setPreset), without requiring a resetFilters() call.
  // Bug: _movingCount never decreased, so the average kept including more samples
  // than the newly configured window size.
  {
    PulseWetProbe shrinkProbe;
    shrinkProbe.enableBoardDefaults();
    if (!shrinkProbe.beginTwoPlate(0, 7, 10)) return fail(30);
    shrinkProbe.acquisition().setDummyReads(0).setBurstSamples(1);
    shrinkProbe.filters()
        .setPreset(PwpFilterPreset::RAW)
        .setMovingAverageWindow(8)
        .setMedianEnabled(false)
        .setEmaAlpha(100);
    shrinkProbe.calibrateDryWet(0, 1023);
    // Fill the 8-sample window with 800s
    const int fillHigh[] = {800,800,800,800,800,800,800,800};
    pwpHostSetAnalogSequence(fillHigh, sizeof(fillHigh)/sizeof(fillHigh[0]));
    for (int i = 0; i < 8; ++i) (void)shrinkProbe.read();
    pwpHostClearAnalogSequence();
    // Shrink window to 4 WITHOUT calling resetFilters()
    shrinkProbe.filters().setMovingAverageWindow(4);
    // Now feed 4 zeros. If _movingCount was stuck at 8, the average would be
    // (800*4 + 0*4)/8 = 400 instead of 0.
    const int fillZero[] = {0,0,0,0};
    pwpHostSetAnalogSequence(fillZero, sizeof(fillZero)/sizeof(fillZero[0]));
    PwpReading shrinkLast = {};
    for (int i = 0; i < 4; ++i) shrinkLast = shrinkProbe.read();
    pwpHostClearAnalogSequence();
    if (shrinkLast.filtered != 0) return fail(31);
  }

#if PWP_ENABLE_JSON
  char json[PWP_JSON_BUFFER_SIZE];
  if (r.toJson(json, sizeof(json)) == 0) return fail(5);
  if (std::strstr(json, "\"quality\"") == nullptr) return fail(5);
#endif

  PwpBoardCapabilities caps = probe.capabilities();
  if (!caps.hasAnalogRead) return fail(6);
  return r.qualityScore > 0 ? 0 : fail(1);
}
