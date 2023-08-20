#include "HMI.h"

// ##########
// Page 1
// ##########
// Show Graphics on mainscreen;
// commit the byModeActual and return the updated operating mode;
auto showMainscreen(byte byMode, byte &byModeOld) -> byte{
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_BLACK);
  byPageId = PAGE_MAIN;

  if (byMode == BATTERY_MODE && byModeOld != BATTERY_MODE) {
    drawGradientBackground();
    drawImage(cSolarPanelOn, SYMBOL_HEIGHT, SYMBOL_WIDTH, 30, 150);
    drawImage(cBatteryOn, SYMBOL_HEIGHT, SYMBOL_WIDTH, 127, 150);
    drawConnectionLine(byMode);
    drawImage(cValueBox, VALUEBOX_HEIGHT, VALUEBOX_WIDTH, 59, 55);
    drawImage(cInverterOff, INVERTER_HEIGHT_WIDTH, INVERTER_HEIGHT_WIDTH, 224, 150);
    byModeOld  = BATTERY_MODE;
  } else if (byMode == INVERTER_MODE && byModeOld != INVERTER_MODE) {
    drawGradientBackground();
    drawImage(cSolarPanelOn, SYMBOL_HEIGHT, SYMBOL_WIDTH, 30, 150);
    drawImage(cInverterOn, INVERTER_HEIGHT_WIDTH, INVERTER_HEIGHT_WIDTH, 224, 150);
    drawConnectionLine(byMode);
    drawImage(cValueBox, VALUEBOX_HEIGHT, VALUEBOX_WIDTH, 107, 55);
    drawImage(cBatteryOff, SYMBOL_HEIGHT, SYMBOL_WIDTH, 127, 150);
    byModeOld  = INVERTER_MODE;
  }
}

// show Menu bar at the top of the screen
auto showMenuBar(byte byPageId) -> void {
  if (byPageId == PAGE_MAIN)
  {
    drawImage(cBtnSettings, SETTINGS_HEIGHT, SETTINGS_WIDTH, 250, 10);
    // TODO: later add more buttons / functions
  }
}


// ##########
// Page 2
// ##########
// Settings Menu page
auto showSettingsMenu() -> void {
  tft.setTextSize(2);
  drawGradientBackground();
  drawImage(cSettingsMenuBtn, SETTINGSMENUBTN_HEIGHT, SETTINGSMENUBTN_WIDTH, 30, 20);
  tft.setCursor(70, 35);
  tft.print("Datum & Uhrzeit");
  drawImage(cSettingsMenuBtn, SETTINGSMENUBTN_HEIGHT, SETTINGSMENUBTN_WIDTH, 30, 70);
  tft.setCursor(90, 85);
  tft.print("Kalibrierung");
  drawImage(cSettingsMenuBtn, SETTINGSMENUBTN_HEIGHT, SETTINGSMENUBTN_WIDTH, 30, 120);
  tft.setCursor(95, 135);
  tft.print("Steuerlogik");
  // drawImage(cPageUp, FOOTER_BTN_HEIGHT, FOOTER_BTN_WIDTH, 89, 190);
  drawImage(cHomeBtn, FOOTER_BTN_HEIGHT, FOOTER_BTN_WIDTH, 140, 190);
  // drawImage(cPageDown, FOOTER_BTN_HEIGHT, FOOTER_BTN_WIDTH, 191, 190);

}


// ##########
// Page 3
// ##########
// Control Logic Settings page
auto showCtrlLogicSettings() -> void {
  drawGradientBackground();


  // drawImage(cPageUp, FOOTER_BTN_HEIGHT, FOOTER_BTN_WIDTH, 89, 190);
  drawImage(cHomeBtn, FOOTER_BTN_HEIGHT, FOOTER_BTN_WIDTH, 140, 190);
  // drawImage(cPageDown, FOOTER_BTN_HEIGHT, FOOTER_BTN_WIDTH, 191, 190);


}