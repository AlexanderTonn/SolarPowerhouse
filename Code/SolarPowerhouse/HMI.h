#ifndef HMI_H
#define HMI_H
#include "pinDefinitions.h"
#include "HMI_graphics.h"
#include "ADC.h"
#include "controlLogic.h"
#include "Util.h"
#include <array>

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

byte byModeActual = 0; // display in BATTERY- or INVERTER_MODE
byte byModeOld = 0;    // Detecting whether Mode has been changed
bool xReturnFromOtherPage = false; // necessary for reloading the mainscreen after settings menu

enum pages{
    SPLASH,
    MAIN,
    SETTINGS,
    SETT_PV_SWITCHING1,
    SETT_PV_SWITCHING2,
    NUMPAD
    };

byte byPageId = 0;    // PageId for Future enhancements
byte byPageIdOld = 0; // Detecting whether page has been changed
byte byPageIdAfterNumpad = 0; // necessary for reloading the screen changing values

enum class guiElement{
  CHECKBOX,
  TEXT_INPUT,
  NOT_USED
};

uint32_t uiTimeValueElapsed = 0;    // elapsed Time for value refreshing
uint16_t uiPageRefreshTime = 100;   // Settings option: Target time (ms) for refreshing Page screens
uint16_t uiValueRefreshTime = 1000; // Settings option: Target time (ms) for refreshing values on HMI
uint16_t uiStandbyTargetTime = 20;  // TODO! Settings option: Target time (s) for standby mode, make it changable later
uint8_t uiTftBrightness = 75;      // Settings option: brightness of TFT
uint32_t uiTimeWithNoTouch = 0;     // Time since last touch



constexpr static uint16_t TOUCH_X_MIN = 153;
constexpr static uint16_t TOUCH_X_MAX = 3767;
constexpr static uint16_t TOUCH_Y_MIN = 231;
constexpr static uint16_t TOUCH_Y_MAX = 3868;

auto lcdInit() -> void;
auto hmiLoop() -> void; // Call HMI Stuff here
auto StandbyHandler(uint32_t &uiElapsed, uint32_t uiTarget, bool xTouched) -> void;
auto drawGradientBackground() -> void;
auto showMainscreen() -> void;
auto showSettingsMenu() -> void;

String sVariableName = ""; // have to be written with the name of the variable which have to be edited

auto drawNumpad(String &string) -> void;
auto numpadTouch() -> byte;
auto numPadHandler() -> void; 

auto showValues(byte byPage, byte byOption) -> void;
auto drawConnectionLine(byte byMode) -> void;
auto drawImage(const char *cImgArray, 
              uint16_t uiHeight, 
              uint16_t uiWidth, 
              uint16_t uiX, 
              uint16_t uiY) -> void;
auto convertPixelToColor(uint8_t pixel) -> uint16_t;
auto showMenuBar(byte byPageId) -> void;
auto detectTouch(uint16_t x, uint16_t y, byte &byPageId) -> void;
auto hmiTouch() -> void;
auto printText(uint8_t uiTextSize, 
              uint16_t uiColor, 
              String sString, 
              uint16_t uiX, 
              uint16_t uiY ) -> void;
auto changeCheckbox(bool xCurrentState) -> bool;
auto showCheckboxState(bool xVariable, 
                      uint16_t uiX, 
                      uint16_t uiY) -> void;

template<typename T1, typename T2, typename T3, typename T4>
auto drawSettingsPage(String sLabel,
                  String sLine1, guiElement element1, T1 var1,
                  String sLine2, guiElement element2, T2 var2,
                  String sLine3, guiElement element3, T3 var3,
                  String sLine4, guiElement element4, T4 var4,
                  bool xPrevPage, bool xNextPage) -> void;

auto touchSettingsPage(uint16_t x, uint16_t y,
                      guiElement element1,
                      guiElement element2,
                      guiElement element3,
                      guiElement element4,
                      bool xSwitchPage) -> byte;

template<typename T>
auto updateValue(T T_variable, String sUnit, uint16_t uiLength, uint8_t uiTextSize, uint16_t uiColor, uint16_t uiX, uint16_t uiY) -> void;

auto touchArea(int16_t x, int16_t y ,uint16_t xStart,uint16_t yStart,uint16_t xEnd,uint16_t yEnd) -> bool;



#endif
