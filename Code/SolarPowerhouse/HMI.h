#ifndef HMI_H
#define HMI_H
#include "pinDefinitions.h"
#include "HMI_graphics.h"
#include "SCT013.h"
#include "controlLogic.h"
#include "Util.h"

// https://github.com/HWHardsoft/Arduino-MKR-Codelock
// https://github.com/KrisKasprzak/Adafruit_ILI9341_Menu/blob/main/Examples/Mega2560/Mega2560_Menu_TouchInput/Mega2560_Menu_TouchInput.ino
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h> // https://github.com/PaulStoffregen/XPT2046_Touchscreen

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

XPT2046_Touchscreen touch(TOUCH_CS);

// LCD Variables
// Image -> Array Converter: https://notisrac.github.io/FileToCArray/
// array size is 4489

constexpr static uint8_t BATTERY_MODE = 1;
constexpr static uint8_t INVERTER_MODE = 2;

byte byModeActual = 0; // display in BATTERY- or INVERTER_MODE
byte byModeOld = 0;    // Detecting whether Mode has been changed

constexpr static uint8_t PAGE_MAIN = 1;
constexpr static uint8_t PAGE_SETTINGS = 2;
constexpr static uint8_t PAGE_SETT_PV_SWITCHING = 3;

byte byPageId = 0;    // PageId for Future enhancements
byte byPageIdOld = 0; // Detecting whether page has been changed

uint64_t uiTimePageElasped = 0;     // elapsed Time for Page refreshing
uint64_t uiTimeValueElapsed = 0;    // elapsed Time for value refreshing
uint16_t uiPageRefreshTime = 100;   // Target time (ms) for refreshing Page screens
uint16_t uiValueRefreshTime = 1000; // Target time (ms) for refreshing values on HMI

constexpr static uint16_t TOUCH_X_MIN = 153;
constexpr static uint16_t TOUCH_X_MAX = 3767;
constexpr static uint16_t TOUCH_Y_MIN = 231;
constexpr static uint16_t TOUCH_Y_MAX = 3868;

auto lcdInit() -> void;
auto hmiLoop() -> void; // Call HMI Stuff here
auto lcdStandbyHandler() -> void;
auto drawGradientBackground() -> void;
auto showMainscreen(byte byMode, byte &byModeOld) -> byte;
auto showValues(byte byPage, byte byOption) -> void;
auto drawConnectionLine(byte byMode) -> void;
auto drawImage(const char *cImgArray, uint16_t uiHeight, uint16_t uiWidth, uint16_t uiX, uint16_t uiY) -> void;
auto convertPixelToColor(uint8_t pixel) -> uint16_t;
auto showMenuBar(byte byPageId) -> void;
auto detectTouch(uint16_t x, uint16_t y, byte &byPageId) -> void;
auto showSettingsMenu() -> void;
auto showSett_PvPageSwitching() -> void;
auto printText(uint8_t uiTextSize, uint16_t uiColor, String sString, uint16_t uiX, uint16_t uiY ) -> void;
auto changeCheckbox(bool xCurrentState) -> bool;
auto showCheckboxState(bool xVariable, uint16_t uiX, uint16_t uiY) -> void;

template<typename T>
auto updateValue(T T_variable, String sUnit, uint16_t uiLength, uint8_t uiTextSize, uint16_t uiColor, uint16_t uiX, uint16_t uiY) -> void;
auto touchArea(int16_t x, int16_t y ,uint16_t xStart,uint16_t yStart,uint16_t xEnd,uint16_t yEnd) -> bool;

#endif
