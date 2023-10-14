#include "ADC.h"
#include "Util.h"
// SOURCES: https://www.poweruc.pl/blogs/news/non-invasive-sensor-yhdc-sct013-000-ct-used-with-arduino-sct-013

// Current Measuring with ADS1115

auto adsInit() -> void
{
  //Wire.setClock(4000000);  // 400kHz I2C clock. Comment this line if having compilation difficulties 
  
  ads1.setGain(GAIN_ONE);
  ads1.begin(0x48);       // default address
  ads1.setDataRate(RATE_ADS1115_860SPS);
  
  ads2.setGain(GAIN_ONE);
  ads2.begin(0x4A);  
  ads2.setDataRate(RATE_ADS1115_860SPS);
}

auto sctMeasuringAds1115(Adafruit_ADS1115 ads, adsInput input, uint16_t &uiCurrentSamples ,uint16_t uiTargetSamples, float fCurrentValue) -> float{
  float fVolt = 0.0;
  float fCurrent = 0.0;
  static float fBuffer;


  // decide which input to use and read voltage
  switch (input)
  {
  case adsInput::ADS_INPUT_0_1:
    fVolt = ads.readADC_Differential_0_1();
    fCurrent = fmap (fVolt, iAdc1_01Low, iAdc1_01High, iAdc1_01ScaledLow, iAdc1_01ScaledHigh);
    break;
  case adsInput::ADS_INPUT_2_3:
    fVolt = ads.readADC_Differential_2_3();
    fCurrent = fmap (fVolt, iAdc1_23Low, iAdc1_23High, iAdc1_23ScaledLow, iAdc1_23ScaledHigh);
    break;
  default:
    Serial.println("Error: Wrong Input for ADS1115");
    break;
  }

  // do calculation 
    fBuffer += sq(fCurrent);
    uiCurrentSamples++;

  // return result if enough samples are collected
  if(uiCurrentSamples >= uiTargetSamples){
    auto auResult = sqrt(fBuffer / uiCurrentSamples); 
    uiCurrentSamples = 0;
    fBuffer = 0.0;
    return auResult;
  }else 
    return fCurrentValue;
  } 


auto voltMeasuringAds1115(Adafruit_ADS1115 ads, adsInput input, voltRange vRange) -> float
{
  float fVolt = 0.0, fResult = 0.0;

  // decide which input to use and read voltage
  switch (input)
  {
  case adsInput::ADS_INPUT_0_1:
    fVolt = ads.readADC_Differential_0_1();
    break;
  case adsInput::ADS_INPUT_2_3:
    fVolt = ads.readADC_Differential_2_3();
    break;
  default:
    Serial.println("Error: Wrong Input for ADS1115");
    break;
  }

  // decide which range to use
  switch (vRange)
  {
  case voltRange::VOLT_0_30:
    fResult = fmap(fVolt, iAdc2_01Low, iAdc2_01High, iAdc2_01ScaledLow, iAdc2_01ScaledHigh);
    break;
  case voltRange::VOLT_0_50:
    fResult = fmap(fVolt, iAdc2_23Low, iAdc2_01High, iAdc2_23ScaledLow, iAdc2_23ScaledHigh);
    break;
  default:
    Serial.println("Error: Wrong Volt Range for ADS1115");
    break;
  }

  return fResult;
}
