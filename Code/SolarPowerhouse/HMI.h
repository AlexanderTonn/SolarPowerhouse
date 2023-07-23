#ifndef HMI_H
#define HMI_H
#include "pinDefinitions.h"

// https://github.com/HWHardsoft/Arduino-MKR-Codelock
// https://github.com/KrisKasprzak/Adafruit_ILI9341_Menu/blob/main/Examples/Mega2560/Mega2560_Menu_TouchInput/Mega2560_Menu_TouchInput.ino
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>  // https://github.com/PaulStoffregen/XPT2046_Touchscreen

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
XPT2046_Touchscreen touch(TOURCH_CS);

// LCD Variables
// Image -> Array Converter: https://notisrac.github.io/FileToCArray/
// array size is 4489

#define BATTERY_MODE 1
#define INVERTER_MODE 2
byte byModeActual = 0;
byte byModeOld = 0;  // Detectin whether Mode have changed

#define PAGE_MAIN 1
byte byPageId = 0;  // PageId for Future enhancements

void lcdInit();
void lcdStandbyHandler();
void drawGradientBackground();
byte showMainscreen(byte byMode, byte &byModeOld);
void showValues(byte byPage, byte byMode);
void drawConnectionLine(byte byMode);
void drawImage(const char* cImgArray, byte byHeight, byte byWidth, byte byX, byte byY);
uint16_t convertPixelToColor(uint8_t pixel);

#endif
