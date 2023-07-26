#ifndef SCT013_H
#define SCT013_H

#include <Adafruit_ADS1X15.h>     // https://github.com/adafruit/Adafruit_ADS1X15/blob/master/examples/differential/differential.ino
Adafruit_ADS1115 ads1;  // ADC for PV current
Adafruit_ADS1115 ads2;  // ADC for voltage measuring

// SCT013 Variables
const float fFactor = 50.0;  //50A/1V
const float fMultiplier = 0.0625F;

float fCurrMppt = 0.0;      // Current to MPPT Charger
float fCurrInverter = 0.0;  // Current to DC/AC Inverter

auto sctMeasuringAds1115(String sAdsInput) -> float;

#endif