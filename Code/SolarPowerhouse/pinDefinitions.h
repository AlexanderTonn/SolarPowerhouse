#ifndef PIN_DEFINITION_H
#define PIN_DEFINITION_H

// ###############
// IO Config
// ###############
#if defined(ARDUINO_SAMD_MKRWIFI1010)
#define TFT_DC 0
#define TFT_CS 1
#define TOUCH_IRQ 2
#define TOURCH_CS 3
#define K1_INVERTER_DC 4
#define K2_INVERTER_AC 5
#define K3_MPPT_CHARGER 6
#define K4_FANS 7
#define TFT_SD_CS 13


#define TFT_LED A0
#define BATTERY_VOLTAGE A1  // TODO: SCT013 for PV Panel pair 1

#else
#error Selected Board have no supported Pin configuration
#endif

#endif