#include "ADC.h"
#include "Util.h"
// SOURCES: https://www.poweruc.pl/blogs/news/non-invasive-sensor-yhdc-sct013-000-ct-used-with-arduino-sct-013

// Current Measuring with ADS1115

auto adsInit() -> void
{
  
  delay(500);       // 100ms was not enough for init
  //Wire.beginTransmission(0x04); // enabling highspeed mode
  //Wire.endTransmission();
  Wire.setClock(4000000);  // 400kHz I2C clock. Comment this line if having compilation difficulties 

  ads1.setGain(GAIN_TWO);
  ads1.begin();       // default address
  ads1.setDataRate(RATE_ADS1115_860SPS);

  ads2.setGain(GAIN_TWO);
  ads2.begin(0x4A);  
  ads2.setDataRate(RATE_ADS1115_16SPS);
}

auto sctMeasuringAds1115(Adafruit_ADS1115 ads, adsInput input, uint16_t &uiCurrentSamples ,uint16_t uiTargetSamples, float fCurrentValue) -> float{
  constexpr static float FACTOR = 50;          //50A/1V
  constexpr static float MULTIPLIER = 0.0625F;  // 0.5mV per bit
  float fVolt = 0.0;
  float fCurrent = 0.0;
  static float fBuffer;


  // decide which input to use and read voltage
  switch (input)
  {
  case adsInput::ADS_INPUT_0_1:
    fVolt = ads.readADC_Differential_0_1() * MULTIPLIER;
    break;
  case adsInput::ADS_INPUT_2_3:
    fVolt = ads.readADC_Differential_2_3() * MULTIPLIER;
    break;
  default:
    Serial.println("Error: Wrong Input for ADS1115");
    break;
  }

  // do calculation 

    fCurrent = fVolt * FACTOR;
    Serial.println("fCurrent: " + String(fCurrent));
    fCurrent /= 1000.0; 
    fBuffer += sq(fCurrent);
    uiCurrentSamples++;

    // Serial.println(ads1.getDataRate());


  
  
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
  constexpr static float MULTIPLIER = 0.0625F;  
  float fVolt = 0.0;
  float fResult = 0.0;

  // decide which input to use and read voltage
  switch (input)
  {
  case adsInput::ADS_INPUT_0_1:
    fVolt = ads.readADC_Differential_0_1() * MULTIPLIER;
    break;
  case adsInput::ADS_INPUT_2_3:
    fVolt = ads.readADC_Differential_2_3() * MULTIPLIER;
    break;
  default:
    Serial.println("Error: Wrong Input for ADS1115");
    break;
  }

  // decide which range to use
  switch (vRange)
  {
  case voltRange::VOLT_0_30:
    fResult = fmap(fVolt, 0.0, 3000.0, 0.0, 30.0);
    break;
  case voltRange::VOLT_0_50:
    fResult = fmap(fVolt, 0.0, 3000.0, 0.0, 50.0);
    break;
  default:
    Serial.println("Error: Wrong Volt Range for ADS1115");
    break;
  }

  return fResult;
}
