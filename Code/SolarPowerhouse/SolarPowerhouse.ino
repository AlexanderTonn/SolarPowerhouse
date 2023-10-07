
#include "HMI_graphics.h"
#include "controlLogic.h"
#include "HMI.h"
#include "ADC.h"
#include "Util.h"

void setup()
{
  pinMode(TFT_CS, OUTPUT);
  pinMode(TFT_DC, OUTPUT);
  pinMode(TFT_SD_CS, OUTPUT);
  pinMode(TFT_LED, OUTPUT);
  pinMode(K1_INVERTER_DC, OUTPUT);
  pinMode(K2_INVERTER_AC, OUTPUT);
  pinMode(K3_MPPT_CHARGER, OUTPUT);
  pinMode(TOUCH_CS, OUTPUT);
  pinMode(TOUCH_IRQ, INPUT);

  SPI.begin();
  // SPI.beginTransaction(SPISettings(48000000, MSBFIRST, SPI_MODE0)); // 48MHz clock speed, more isn't possible with wifi1010
  delay(1000);

  Serial.begin(uiBaudrate);
  delay(1000); // 100ms was not enough for init
  adsInit();

  Wire.begin();

  rtcInit();
  // ds3231SetupHandler();

  lcdInit();

  drawGradientBackground();

  byPageId = MAIN;
}

void loop()
{
  // READING INPUTS

  if (!xSett_ignoreCurrent)
  {
    if (byModeActual == INVERTER_MODE)
    {
      fCurrInverter = sctMeasuringAds1115(ads1, adsInput::ADS_INPUT_0_1, uiInverterSamples, 100, fCurrInverter);
    }
    else if (byModeActual == BATTERY_MODE)
    {
      fCurrMppt = sctMeasuringAds1115(ads1, adsInput::ADS_INPUT_2_3, uiMpptSamples, 100, fCurrMppt);
    }
  }else
  {
    fCurrInverter = 0.0;
    fCurrMppt = 0.0;
  }

  fBatteryVoltage = voltMeasuringAds1115(ads2, adsInput::ADS_INPUT_0_1, voltRange::VOLT_0_30);
  fSolarVoltage = voltMeasuringAds1115(ads2, adsInput::ADS_INPUT_2_3, voltRange::VOLT_0_50);

  // CONTROL LOGIC
  controlLogic();

  // Set Outputs
  if (xPv1OnInverter)
    pv1OnInverter();
  if (xPv1OnMppt)
    pv1OnMppt();

  // HMI
  hmiTouch();
  hmiLoop();
}
