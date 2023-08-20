#ifndef PIN_DEFINITION_H
#define PIN_DEFINITION_H

// ###############
// IO Config
// ###############
#if defined(ARDUINO_SAMD_MKRWIFI1010)
constexpr static uint8_t TFT_DC = 0;
constexpr static uint8_t TFT_CS = 1;
constexpr static uint8_t TOUCH_IRQ = 2;
constexpr static uint8_t TOUCH_CS = 3;
constexpr static uint8_t K1_INVERTER_DC = 4;
constexpr static uint8_t K2_INVERTER_AC = 5;
constexpr static uint8_t K3_MPPT_CHARGER = 6;
constexpr static uint8_t K4_FANS = 7;
constexpr static uint8_t TFT_SD_CS = 13;

constexpr static uint8_t TFT_LED = A0;
//#define TOUCH_DO A1     // TODO: rewire to MISO
//#define TOUCH_DIN A2    // TODO: rewire to MOSI
//#define TOUCH_CLK A3    // TODO: rewire to SCK



#else
#error Selected Board have no supported Pin configuration
#endif

#endif