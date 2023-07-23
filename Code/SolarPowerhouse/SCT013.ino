// Current Measuring with ADS1115
float sctMeasuringAds1115(String sAdsInput) {
  float fVolt = 0.0;
  float fCurrent = 0.0;
  float fSum = 0.0;
  uint32_t uiTimer = millis();

  if (sAdsInput == "01") {
    while (millis() - uiTimer < 1000) {
      fVolt = ads1.readADC_Differential_0_1() * fMultiplier;
      fCurrent = fVolt * fFactor;
      fCurrent /= 1000.0;
    }
  } else if (sAdsInput == "23") {
    while (millis() - uiTimer < 1000) {
      fVolt = ads1.readADC_Differential_2_3() * fMultiplier;
      fCurrent = fVolt * fFactor;
      fCurrent /= 1000.0;
    }
  }
  return fCurrent;
}