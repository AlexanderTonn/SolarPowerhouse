#ifndef PIN_DEFINITION_H
#define PIN_DEFINITION_H

// ###############
// IO Config
// ###############
#if defined(ARDUINO_OPTA)

// ############
// Currently not used:
// D3 && LED_D3
// BTN_USER
// All Inputs from A0 to A7
// ############

constexpr static uint8_t K1_INVERTER_DC_SIDE = D0; // Relay for Inverter DC
constexpr static uint8_t K1_LED = LED_D0; // LED for Relay K1
constexpr static uint8_t K2_MPPT_CHARGER = D1; // Relay for Inverter AC
constexpr static uint8_t K2_LED = LED_D1; // LED for Relay K2
constexpr static uint8_t K3_INVERTER_AC_SIDE = D2; // Relay for MPPT Charger
constexpr static uint8_t K3_LED = LED_D2; // LED for Relay K3

#else
#error Selected Board have no supported Pin configuration
#endif

#endif