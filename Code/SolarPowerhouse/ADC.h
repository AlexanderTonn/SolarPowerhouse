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

int16_t iAdc2_01Low = 0;         // Threshold for raw value of differential input Battery
int16_t iAdc2_01High = 24613;    // Threshold for raw value of differential input Battery
int16_t iAdc2_23Low = 0;         // Threshold for raw value of differential input PV 
int16_t iAdc2_23High = 24613;     // Threshold for raw value of differential input PV 
int8_t iAdc2_01ScaledLow = 0;   // Threshold for scaled value of differential input Battery
int8_t iAdc2_01ScaledHigh = 30; // Threshold for scaled value of differential input Battery
int8_t iAdc2_23ScaledLow = 0;   // Threshold for scaled value of differential input PV
int8_t iAdc2_23ScaledHigh = 50; // Threshold for scaled value of differential input PV

int16_t iAdc1_01Low = 0;         // Threshold for raw value of HSTS016L for Inverter
int16_t iAdc1_01High = 19989;    // Threshold for raw value of HSTS016L for Inverter
int16_t iAdc1_23Low = 0;         // Threshold for raw value of HSTS016L for MPPT
int16_t iAdc1_23High = 19989;     // Threshold for raw value of HSTS016L for MPPT
int8_t iAdc1_01ScaledLow = 0;   // Threshold for scaled value of HSTS016L for Inverter
int8_t iAdc1_01ScaledHigh = 30; // Threshold for scaled value of HSTS016L for Inverter
int8_t iAdc1_23ScaledLow = 0;   // Threshold for scaled value of HSTS016L for MPPT
int8_t iAdc1_23ScaledHigh = 30; // Threshold for scaled value of HSTS016L for MPPT

auto adsInit() -> void;
auto sctMeasuringAds1115(Adafruit_ADS1115 ads, adsInput input, uint16_t &uiCurrentSamples ,uint16_t uiTargetSample, float fCurrentValue) -> float;
auto voltMeasuringAds1115(Adafruit_ADS1115 ads, adsInput input, voltRange vRange) -> float;

#endif