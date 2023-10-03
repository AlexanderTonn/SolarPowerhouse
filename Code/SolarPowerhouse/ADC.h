#ifndef ADC_H
#define ADC_H

#include <Adafruit_ADS1X15.h>     // https://github.com/adafruit/Adafruit_ADS1X15/blob/master/examples/differential/differential.ino

Adafruit_ADS1115 ads1;  // ADC for PV current
Adafruit_ADS1115 ads2;  // ADC for voltage measuring

enum class adsInput{
    ADS_INPUT_0_1,
    ADS_INPUT_2_3,
    };
enum class voltRange{
    VOLT_0_30,
    VOLT_0_50
};

float fCurrMppt = 0.0;                  // Current to MPPT Charger
float fCurrInverter = 0.0;              // Current to DC/AC Inverter
uint16_t uiInverterSamples = 0;        // Counter for Inverter Samples
uint16_t uiMpptSamples = 0;           // Counter for MPPT Samples

auto adsInit() -> void;
auto sctMeasuringAds1115(Adafruit_ADS1115 ads, adsInput input, uint16_t &uiCurrentSamples ,uint16_t uiTargetSample, float fCurrentValue) -> float;
auto voltMeasuringAds1115(Adafruit_ADS1115 ads, adsInput input, voltRange vRange) -> float;

#endif