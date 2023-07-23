
#include "HMI_graphics.h"
#include "controlLogic.h"
#include "HMI.h"
#include "RTC.h"
#include "SCT013.h"
#include "Util.h"

void setup() {
  pinMode(TFT_CS, OUTPUT);
  pinMode(TFT_DC, OUTPUT);
  pinMode(TFT_SD_CS, OUTPUT);  // playing safe that RX Pin 12 is defined as output pin for chip select
  pinMode(TFT_LED, OUTPUT);    // TFT Brightness
  pinMode(K1_INVERTER_DC, OUTPUT);
  pinMode(K2_INVERTER_AC, OUTPUT);
  pinMode(K3_MPPT_CHARGER, OUTPUT);
  pinMode(K4_FANS, OUTPUT);

  pinMode(BATTERY_VOLTAGE, INPUT);

  Serial.begin(uiBaudrate);
  delay(1000);  // 100ms was not enough for init
  Wire.begin();
  delay(1000);       // 100ms was not enough for init
  ads1.begin(0x48);  // default address
  ads2.begin(0x49);  // TODO: if i insert the module, the code will not execute

  rtcInit();
  // ds3231SetupHandler();

  lcdInit();

  drawGradientBackground();

  byPageId = PAGE_MAIN;
}

void loop() {
  // READING INPUTS
  fCurrMppt = sctMeasuringAds1115("01");
  fCurrInverter = sctMeasuringAds1115("23");

  xTrigBatteryFull = edgeDetection(fBatteryVoltage, fTrigBatteryFullPrevious, RISING_EDGE, fBatterySoCVoltage[8]);     //Battery is full after reaching 100%
  xTrigBatteryEmpty = edgeDetection(fBatteryVoltage, fTrigBatteryEmptyPrevious, FALLING_EDGE, fBatterySoCVoltage[4]);  // Battery should charged if dropped down to 50%

  // CONTROL LOGIC
  // Set Relays on Mppt if day changed
  xDateChanged = rtcCheckDayChange();
  if (xDateChanged == true && fCurrMppt <= fSwitchCurrentTarget && fCurrInverter <= fSwitchCurrentTarget || xFirstCycle == true) {
    relayOnMppt();
    byModeActual = BATTERY_MODE;
    xFirstCycle = false;
  }

  // Switching from mppt to inverter
  if (xTrigBatteryFull == true || xSwitchLaterOnInverter == true) {
    if (fCurrMppt <= fSwitchCurrentTarget) {
      relayOnInverter();
      byModeActual = INVERTER_MODE;
      xSwitchLaterOnInverter = false;
    } else
      xSwitchLaterOnInverter = true;
  }
  // Switching from Invertwe to mppt charger
  // if current is certainly to high then try to switch later if the current is low
  else if (xTrigBatteryEmpty == true || xSwitchLaterOnMppt == true) {
    if (fCurrInverter <= fSwitchCurrentTarget) {
      relayOnMppt();
      byModeActual = BATTERY_MODE;
      xSwitchLaterOnMppt = false;
    } else
      xSwitchLaterOnMppt = true;
  }

  // TODO: DO Reload if Value have changed to prevent "flickering" reloading of the value or only do this each seconds 

  bool x1 = false;
  x1 = functionTrigger(uiMyMillis,1000);
  if(x1)
    showValues(PAGE_MAIN, byModeActual);
  if(x1 && byPageId == PAGE_MAIN)
    showMainscreen(byModeActual, byModeOld);

  delay(1000);
}

