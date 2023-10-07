#include "HMI.h"

// ##########
// Page 1
// ##########
// Show Graphics on mainscreen;
// commit the byModeActual and return the updated operating mode;
auto showMainscreen() -> void{
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_BLACK);
  byPageId = MAIN;

  if (byModeActual == BATTERY_MODE && (byModeOld != BATTERY_MODE || xReturnFromOtherPage == true))
  {
    drawGradientBackground();
    drawImage(cSolarPanelOn, SYMBOL_HEIGHT, SYMBOL_WIDTH, 30, 150);
    drawImage(cBatteryOn, SYMBOL_HEIGHT, SYMBOL_WIDTH, 127, 150);
    drawConnectionLine(byModeActual);
    drawImage(cInverterOff, INVERTER_HEIGHT_WIDTH, INVERTER_HEIGHT_WIDTH, 224, 150);
    if(!xSett_ignoreCurrent)
      drawImage(cValueBox, VALUEBOX_HEIGHT, VALUEBOX_WIDTH, 59, 55);
    byModeOld = byModeActual;
  } else if (byModeActual == INVERTER_MODE && (byModeOld != INVERTER_MODE || xReturnFromOtherPage == true))
  {
    drawGradientBackground();
    drawImage(cSolarPanelOn, SYMBOL_HEIGHT, SYMBOL_WIDTH, 30, 150);
    drawImage(cInverterOn, INVERTER_HEIGHT_WIDTH, INVERTER_HEIGHT_WIDTH, 224, 150);
    drawConnectionLine(byModeActual);
    drawImage(cBatteryOff, SYMBOL_HEIGHT, SYMBOL_WIDTH, 127, 150);
    if(!xSett_ignoreCurrent)
      drawImage(cValueBox, VALUEBOX_HEIGHT, VALUEBOX_WIDTH, 107, 55);
    byModeOld = byModeActual;
  }
  xReturnFromOtherPage = false; 
}

// show Menu bar at the top of the screen
auto showMenuBar(byte byPageId) -> void {
  if (byPageId == MAIN)
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

// dynamically Page for displaying Settings
// bool xPrevPage, bool xNextPage  == show button for page up/down - detechTouch have to be done seperatly
template<typename T1, typename T2, typename T3, typename T4>
auto drawSettingsPage(String sLabel,
                  String sLine1, guiElement element1, T1 var1,
                  String sLine2, guiElement element2, T2 var2,
                  String sLine3, guiElement element3, T3 var3,
                  String sLine4, guiElement element4, T4 var4,
                  bool xPrevPage, bool xNextPage) -> void
 {
    constexpr static uint8_t  TEXTBOX_HEIGHT = 32;
    constexpr static uint8_t  TEXTBOX_WIDTH = 99;

  drawGradientBackground();


  // Print Text Labels
    printText(3,ILI9341_BLACK,sLabel,10,10);
    printText(2,ILI9341_BLACK,sLine1,10,50);
    printText(2,ILI9341_BLACK,sLine2,10,85);
    printText(2,ILI9341_BLACK,sLine3,10,120);
    printText(2,ILI9341_BLACK,sLine4,10,155);

  // Print GUI Input Elements
  // Line1
    switch (element1)
    {
      case guiElement::CHECKBOX :
      showCheckboxState(var1, 280, 40);
      break;

      case guiElement::TEXT_INPUT: 
      drawImage(cValueboxSettings,TEXTBOX_HEIGHT,TEXTBOX_WIDTH, 213, 40);
      break;

      default: 
      // DO nothing special if wrong / not exsisting enum was passed
      break;
    }
    // Line 2
    switch (element2)
    {
      case guiElement::CHECKBOX :
      showCheckboxState(var2, 280, 75);
      break;

      case guiElement::TEXT_INPUT: 
      drawImage(cValueboxSettings,TEXTBOX_HEIGHT,TEXTBOX_WIDTH, 213, 75);
      break;
      default: 
      // DO nothing special if wrong / not exsisting enum was passed
      break;
    }
    // Line 3
    switch (element3)
    {
      case guiElement::CHECKBOX :
      showCheckboxState(var3, 280, 110);
      break;

      case guiElement::TEXT_INPUT: 
      drawImage(cValueboxSettings,TEXTBOX_HEIGHT,TEXTBOX_WIDTH, 213, 110);
      break;
      default: 
      // DO nothing special if wrong / not exsisting enum was passed
      break;
    }
    // Line 4
    switch (element4)
    {
      case guiElement::CHECKBOX :
      showCheckboxState(var4, 280, 145);
      break;

      case guiElement::TEXT_INPUT: 
      drawImage(cValueboxSettings,TEXTBOX_HEIGHT,TEXTBOX_WIDTH, 213, 145);
      break;
      default: 
      // DO nothing special if wrong / not exsisting enum was passed
      break;
    }

    // Footer
    if(xPrevPage)
      drawImage(cPageUp, FOOTER_BTN_HEIGHT, FOOTER_BTN_WIDTH, 89, 190);
    
    drawImage(cHomeBtn, FOOTER_BTN_HEIGHT, FOOTER_BTN_WIDTH, 140, 190);

    if(xNextPage)
      drawImage(cPageDown, FOOTER_BTN_HEIGHT, FOOTER_BTN_WIDTH, 191, 190);
   
 }


// Show Numpad for changing variables on HMI
 auto drawNumpad(String &string) -> void 
{
  drawGradientBackground();
  // Value box
  printText(2,ILI9341_BLACK, string ,10, 20);
  tft.fillRect(175, 5, 130, 40, ILI9341_BLACK);
  tft.fillRect(177, 7, 126, 36, ILI9341_WHITE);

  tft.drawFastHLine(0,50,320,ILI9341_BLACK);
  tft.drawFastHLine(0,51,320,ILI9341_BLACK);
  tft.drawFastHLine(0,97,241,ILI9341_BLACK);
  tft.drawFastHLine(0,98,241,ILI9341_BLACK);
  tft.drawFastHLine(0,144,320,ILI9341_BLACK);
  tft.drawFastHLine(0,143,320,ILI9341_BLACK);
  tft.drawFastHLine(0,191,241,ILI9341_BLACK);
  tft.drawFastHLine(0,192,241,ILI9341_BLACK);
  tft.drawFastVLine(80,50,240,ILI9341_BLACK);
  tft.drawFastVLine(81,50,240,ILI9341_BLACK);
  tft.drawFastVLine(160,50,240,ILI9341_BLACK);
  tft.drawFastVLine(161,50,240,ILI9341_BLACK);
  tft.drawFastVLine(240,50,240,ILI9341_BLACK);  
  tft.drawFastVLine(241,50,240,ILI9341_BLACK); 

  // ENTER btn
  tft.fillTriangle(260, 117, 270, 107, 270, 127, ILI9341_BLACK);
  tft.drawFastHLine(270, 117, 20, ILI9341_BLACK);
  tft.drawFastHLine(270, 118, 20, ILI9341_BLACK);
  tft.drawFastVLine(290, 68, 50,ILI9341_BLACK); 
  tft.drawFastVLine(291, 68, 50,ILI9341_BLACK); 

  // CLEAR 
  printText(3,ILI9341_BLACK,"CLR",255,181);

  // NUMBERS
  // First column
  printText(3,ILI9341_BLACK,"1",30,64);
  printText(3,ILI9341_BLACK,"4",30,111);
  printText(3,ILI9341_BLACK,"7",30,158);
  printText(3,ILI9341_RED,"X",30,205); // Abort button 
  // second column
  printText(3,ILI9341_BLACK,"2",110,64);
  printText(3,ILI9341_BLACK,"5",110,111);
  printText(3,ILI9341_BLACK,"8",110,158);
  printText(3,ILI9341_BLACK,"0",110,205);
  //third column
  printText(3,ILI9341_BLACK,"3",190,64);
  printText(3,ILI9341_BLACK,"6",190,111);
  printText(3,ILI9341_BLACK,"9",190,158);
  printText(3,ILI9341_BLACK,",",190,205);

}

