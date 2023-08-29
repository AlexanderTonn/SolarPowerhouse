#ifndef PIN_DEFINITION_H
#define PIN_DEFINITION_H

// ###############
// IO Config
// ###############
#if defined(ARDUINO_SAMD_MKRWIFI1010)
constexpr static int8_t TFT_DC = 0; // TFT Data/Command
constexpr static int8_t TFT_CS = 1; // TFT Chip Select
constexpr static uint8_t TOUCH_IRQ = 2; // Touch Interrupt
constexpr static uint8_t TOUCH_CS = 3; // Touch Chip Select
constexpr static uint8_t K1_INVERTER_DC = A1; // Relay for Inverter DC
constexpr static uint8_t K2_INVERTER_AC = A2; // Relay for Inverter AC
constexpr static uint8_t K3_MPPT_CHARGER = A3; // Relay for MPPT Charger
constexpr static uint8_t TFT_SD_CS = A4; // TFT SD Card Chip Select

constexpr static uint8_t TFT_LED = A0; // TFT Backlight






#else
#error Selected Board have no supported Pin configuration
#endif

#endif