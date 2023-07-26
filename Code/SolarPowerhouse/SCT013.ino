#include "SCT013.h"
// SOURCES: https://www.poweruc.pl/blogs/news/non-invasive-sensor-yhdc-sct013-000-ct-used-with-arduino-sct-013

// Current Measuring with ADS1115
auto sctMeasuringAds1115(String sAdsInput) -> float{
  float fVolt = 0.0;
  float fCurrent = 0.0;
  float fSum = 0.0;
  uint16_t uiNumOfSamples = 0; 
  auto uiTimer = millis();

  if (sAdsInput == "01") {
    while (uiNumOfSamples < 100) {
      fVolt = ads1.readADC_Differential_0_1() * fMultiplier;
      fCurrent = fVolt * fFactor;
      fCurrent /= 1000.0; 
      fSum += sq(fCurrent);
      uiNumOfSamples++;
      Serial.print("fVolt: ");
      Serial.println(fVolt);

    }
  } else if (sAdsInput == "23") {
    while (uiNumOfSamples < 100) {
      fVolt = ads1.readADC_Differential_2_3() * fMultiplier;
      fCurrent = fVolt * fFactor;
      fCurrent /= 1000.0; 
      fSum += sq(fCurrent);
      uiNumOfSamples++;
    }
  }

   return fCurrent = sqrt(fSum / static_cast<uint16_t>(uiNumOfSamples)); 
}