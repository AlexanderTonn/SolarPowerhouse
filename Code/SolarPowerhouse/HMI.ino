#include "HMI.h"

// ##################
// LYQUID CRYSTAL SPI CODE
// ##################
auto lcdInit() -> void
{
  Serial.println("Start Init ILI9341");
  tft.begin();
  touch.begin();
  touch.setRotation(3);

  if (!touch.begin())
  {
    Serial.println("Could not start touch controller xpt2046");
  }
  else
  {
    Serial.println("Touch controller xpt2046 started");
  }
  tft.setRotation(3);
  // TODO: make brightness changeable
  // TODO: draw screen in the background and after finishing turn on tft backlight
  analogWrite(TFT_LED, 75);
  Serial.println("ILI9341 and XPT2046 Init done!");
}

auto hmiLoop() -> void
{
  // Show and refresh pages
  static uint32_t uiTimePageElasped = 0;
  bool x1 = functionTrigger(uiTimePageElasped, uiPageRefreshTime);

  // handle numpad
  if(!byPageIdAfterNumpad == 0)
  {
    numPadHandler();
  }

  if (x1)
  {
    if (byPageId == MAIN)
    {
      showMainscreen();
      showMenuBar(byPageId);
    }else
    {
      xReturnFromOtherPage = true; 
    }

    if (byPageId == SETTINGS && byPageId != byPageIdOld)
    {
      showSettingsMenu();
    }
    else if (byPageId == SETT_PV_SWITCHING1 && byPageId != byPageIdOld)
    {
      drawSettingsPage("PV1 Betriebsmodus",
                  "Batteriebetrieb ", guiElement::CHECKBOX, xSett_PVonMppt,
                  "Inverterbetrieb ", guiElement::CHECKBOX, xSett_PVonInverter,
                  "Reset bei Tageswechsel", guiElement::CHECKBOX, xSett_ResetDay,
                  "Schaltabstand", guiElement::TEXT_INPUT, uiSett_PVswitchingDelay,
                  false, true);
    }
    else if(byPageId == SETT_PV_SWITCHING2 && byPageId != byPageIdOld)
    {
      drawSettingsPage("Schalt-Grenzwerte",
                  "PV1 Strom beachten", guiElement::CHECKBOX, xSett_considerCurrent,
                  "PV1 auf Inverter, ab >", guiElement::TEXT_INPUT, fSett_TargetVoltageInverter,
                  "PV1 auf Batt, ab <", guiElement::TEXT_INPUT, fSett_TargetVoltageMppt,
                  "Schalten, bei < ", guiElement::TEXT_INPUT, fSett_SwitchCurrentTarget,
                  true, false);
    }
    if (byPageId != byPageIdOld)
    {
      byPageIdOld = byPageId;
    }
  }

  bool x2 = functionTrigger(uiTimeValueElapsed, uiValueRefreshTime);
  if (x2)
  {
    showValues(byPageId, byModeActual);
  }
}

// TODO: turn off background if no touch input
auto lcdStandbyHandler() -> void
{
  static uint32_t uiTimeStandbyElapsed = 0;
  static bool xStandby = false;

  bool x1 = functionTrigger(uiTimeStandbyElapsed, uiStandbyTargetTime * 1000);

  // fade out HMI if no touch input
  if(xStandby)
  {
    for(auto i = uiTftBrightness; i > 0; i--)
    {
      analogWrite(TFT_LED, i);
      delay(1);
    }
  }

  // TODO: Turn on, if touch was pressed

}

// #######
// ILI9341: Drawing linear gradient
// #######
auto drawGradientBackground() -> void
{
  int numPixels = tft.width();

  // Farbverlauf erstellen
  for (int x = 0; x < numPixels; x++)
  {
    // Linearen Farbwert berechnen
    uint8_t r = map(x, 0, numPixels - 1, 255, 183);
    uint8_t g = map(x, 0, numPixels - 1, 255, 183);
    uint8_t b = map(x, 0, numPixels - 1, 255, 183);

    // Farbe setzen
    uint16_t color = tft.color565(r, g, b);

    // Pixel zeichnen
    tft.drawFastVLine(x, 0, tft.height(), color);
  }
}

// UPDATING Arduino Variables on Display each second
auto showValues(byte byPage, byte byOption) -> void
{
  switch(byPage)
  {
    case MAIN : 
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_BLACK);
    if (byOption == BATTERY_MODE)
    {
      updateValue(fCurrMppt, "A", 7, 2, ILI9341_BLACK, 70, 68);
    }
    else if (byOption == INVERTER_MODE)
    {
      updateValue(fCurrInverter, "A", 7, 2, ILI9341_BLACK, 118, 68);
    }
    updateValue(fBatteryVoltage, "V", 7, 1, ILI9341_BLACK, 140, 220);
    updateValue(fSolarVoltage, "V", 7, 1, ILI9341_BLACK, 44, 220);
    break;

    case SETT_PV_SWITCHING1 :
    updateValue(uiSett_PVswitchingDelay, "s", 5, 2, ILI9341_BLACK, 235, 154);
    break; 

    default: 
    Serial.println("Error: Page not found");
    break;
  }
}

// #######
// ILI9341: Draw shape: inverted U for connectoion line on mainscreen
// #######
// Syntax: tft.drawLine(xStart,yStart,xEnd,yEnd,Color);
auto drawConnectionLine(byte byMode) -> void
{
  // Connection line between SolarPanel and Battery
  if (byMode == BATTERY_MODE)
  {
    // vertical line Solar Panel, width: 3px
    tft.drawLine(62, 150, 62, 100, ILI9341_DARKGREEN);
    tft.drawLine(63, 150, 63, 100, ILI9341_DARKGREEN);
    tft.drawLine(64, 150, 64, 100, ILI9341_DARKGREEN);
    // horizontal line, width: 3px with 1px corner
    tft.drawLine(62, 100, 161, 100, ILI9341_DARKGREEN);
    tft.drawLine(63, 99, 160, 99, ILI9341_DARKGREEN);
    tft.drawLine(64, 98, 159, 98, ILI9341_DARKGREEN);
    // vertical line battery, width: 3px
    tft.drawLine(159, 150, 159, 100, ILI9341_DARKGREEN);
    tft.drawLine(160, 150, 160, 100, ILI9341_DARKGREEN);
    tft.drawLine(161, 150, 161, 100, ILI9341_DARKGREEN);
  }
  else if (byMode == INVERTER_MODE)
  {
    // vertical line Solar Panel, width: 3px
    tft.drawLine(62, 150, 62, 100, ILI9341_DARKGREEN);
    tft.drawLine(63, 150, 63, 100, ILI9341_DARKGREEN);
    tft.drawLine(64, 150, 64, 100, ILI9341_DARKGREEN);
    // horizontal line, width: 3px with 1px corner
    tft.drawLine(62, 100, 261, 100, ILI9341_DARKGREEN);
    tft.drawLine(63, 99, 260, 99, ILI9341_DARKGREEN);
    tft.drawLine(64, 98, 259, 98, ILI9341_DARKGREEN);
    // vertical line battery, width: 3px
    tft.drawLine(259, 150, 259, 100, ILI9341_DARKGREEN);
    tft.drawLine(260, 150, 260, 100, ILI9341_DARKGREEN);
    tft.drawLine(261, 150, 261, 100, ILI9341_DARKGREEN);
  }
}

// #######
// ILI9341: CONVERT char Array to Image on Screen
// #######
// Function for drawing the img
auto drawImage(const char *cImgArray, uint16_t uiHeight, uint16_t uiWidth, uint16_t uiX, uint16_t uiY) -> void
{
  // Transmit picture data onto screen  for each line
  for (int y = 0; y < uiHeight; y++)
  {
    for (int x = 0; x < uiWidth; x++)
    {
      int index = y * uiWidth + x;
      uint8_t pixel = pgm_read_byte(&cImgArray[index]);
      uint16_t color = convertPixelToColor(pixel);
      tft.drawPixel(uiX + x, uiY + y, color);
    }
  }
}
// Converting 8 Bit into 16 bit color
auto convertPixelToColor(uint8_t pixel) -> uint16_t
{
  uint8_t r = (pixel & 0xE0) >> 5;        // Rote Komponente (RRR00000)
  uint8_t g = (pixel & 0x1C) >> 2;        // Grüne Komponente (GGG00)
  uint8_t b = (pixel & 0x03);             // Blaue Komponente (BB)
  r = (r << 5) | (r << 2) | (r >> 1);     // Auf 8 Bit skalieren (RRRRR000)
  g = (g << 5) | (g << 2) | (g >> 1);     // Auf 8 Bit skalieren (GGGGG000)
  b = (b << 6) | (b << 4) | (b << 2) | b; // Auf 8 Bit skalieren (BBBBB)
  return tft.color565(r, g, b);           // 16-Bit-Farbwert zurückgeben
}

auto hmiTouch() -> void 
{
  // Detecting touch
  if(touch.touched())
  {
    TS_Point touchPoint = touch.getPoint();
    touchPoint.x = map(touchPoint.x, TOUCH_X_MIN, TOUCH_X_MAX, tft.width(), 0);
    touchPoint.y = map(touchPoint.y, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, tft.height());
    Serial.println("X: " + String(touchPoint.x) + " Y: " + String(touchPoint.y));

    detectTouch(touchPoint.x, touchPoint.y, byPageId);
  }
}
// detect touch inputs
auto detectTouch(uint16_t x, uint16_t y, byte &byPageId) -> void
{

  if (x == 0 && y == 0)
  {
    return;
  }
  else
  {

    bool xSettingsTouched = touchArea(x, y, 256, 192, 291, 224); // Settings btn:
    bool xConrolLogicTouched = touchArea(x, y, 46, 88, 282, 117);   // Settings: Control Logic btn
    if (byPageId == MAIN && xSettingsTouched)
    {
      byPageId = SETTINGS;
    }
    else if (byPageId == SETTINGS && xConrolLogicTouched)
    {
      byPageId = SETT_PV_SWITCHING1;
    }
    // Settings: PV-Switching Page
    else if (byPageId == SETT_PV_SWITCHING1)
    {
      bool xCb1Touched = touchArea(x, y, 288, 175, 305, 191); // PV on Mppt
      bool xCb2Touched = touchArea(x, y, 288, 141, 305, 163); // PV on Inverter
      bool xCb3Touched = touchArea(x, y, 288, 108, 305, 128); // Reset Day
      bool xTextInTouched = touchArea(x,y , 235, 154, 270, 170); // Text Input

      if (xCb1Touched)
      {
        xSett_PVonMppt = changeCheckbox(xSett_PVonMppt);
        xSett_PVonInverter = false;                   // if PV on Mppt is selected, PV on Inverter is not allowed
        showCheckboxState(xSett_PVonMppt, 280, 40);
        showCheckboxState(xSett_PVonInverter, 280, 75);
        
      }
      else if (xCb2Touched)
      {
        xSett_PVonInverter = changeCheckbox(xSett_PVonInverter);
        xSett_PVonMppt = false;                       // if PV on Inverter is selected, PV on Mppt is not allowed
        showCheckboxState(xSett_PVonMppt, 280, 40);
        showCheckboxState(xSett_PVonInverter, 280, 75);
      }
      else if (xCb3Touched)
      {
        xSett_ResetDay = changeCheckbox(xSett_ResetDay);
        showCheckboxState(xSett_ResetDay, 280, 110);
      }
      else if(xTextInTouched)
      {
        byPageId = NUMPAD;
        byPageIdAfterNumpad = SETT_PV_SWITCHING1;
        sVariableName = "Schaltabstand";
        drawNumpad(sVariableName);
      }
      delay(100); // preventing bouncing, probably you have later a better solution
    }

    // Home Btn pressed
    if (byPageId > 1 && x >= 147 && x <= 183 && y >= 19 && y <= 46)
    {
      byPageId = MAIN;
    }
  }
}


auto printText(uint8_t uiTextSize, uint16_t uiColor, String sString, uint16_t uiX, uint16_t uiY) -> void
{
  tft.setTextSize(uiTextSize);
  tft.setTextColor(uiColor);
  tft.setCursor(uiX, uiY);
  tft.print(sString);
}
// #######
// Function for updating variables on screen
// #######
// T_Variable = Variable to update
// sUnit = Unit of the variable
// sLength = length of the variable (including empty spaces)
// uiTextSize = tft.setTextSize
// uiColor = tft.setTextColor
// uiX = tft.setCursor x coordinate
// uiY = tft.setCursor y coordinate
template <typename T>
auto updateValue(T T_variable, 
                String sUnit, 
                uint16_t uiLength, 
                uint8_t uiTextSize, 
                uint16_t uiColor, 
                uint16_t uiX, 
                uint16_t uiY) -> void
{
  uint8_t uiHeight = 0;
  uint16_t uiX_start = uiX - 1, uiY_start = uiY - 1, uiX_end = 0;

  if (uiTextSize == 1)
  {
    uiX_end = uiLength * 6;
    uiHeight = 10;
  }
  else if (uiTextSize == 2)
  {
    uiX_end = uiLength * uiTextSize * 6;
    uiHeight = 20;
  }
  else
  {
    return;
  }
  tft.fillRect(uiX_start, uiY_start, uiX_end, uiHeight, ILI9341_WHITE);
  tft.setCursor(uiX, uiY);
  tft.setTextSize(uiTextSize);
  tft.setTextColor(uiColor);
  tft.print(String(T_variable) + " " + sUnit);
}

// Function for printing the checkbox depending of the passed variable
auto showCheckboxState(bool xVariable, 
                      uint16_t uiX, 
                      uint16_t uiY) -> void
{
  if (xVariable)
  {
    drawImage(checkbox1, CHECKBOX_HEIGHT_WIDTH, CHECKBOX_HEIGHT_WIDTH, uiX, uiY);
  }
  else
  {
    drawImage(checkbox0, CHECKBOX_HEIGHT_WIDTH, CHECKBOX_HEIGHT_WIDTH, uiX, uiY);
  }
}
// changing the checkbox state and return the decision to the variable
auto changeCheckbox(bool xCurrentState) -> bool
{
  if (xCurrentState)
  {
    return xCurrentState = false;
  }
  else
  {
    return xCurrentState = true;
  }
}

auto touchArea(int16_t x, 
              int16_t y, 
              uint16_t xStart, 
              uint16_t yStart, 
              uint16_t xEnd, 
              uint16_t yEnd) -> bool
{
  if (x >= xStart && x <= xEnd && y >= yStart && y <= yEnd)
  {
    return true;
  }
  else
  {
    return false;
  }
}

auto detectNumPadtouch(uint16_t x, uint16_t y, bool &xEnter) -> void
{
  if (x == 0 && y == 0)
  {
    return;
  }
  else
  {
  }
}

auto numpadTouch() -> byte
{
  TS_Point touchPoint = touch.getPoint();
  touchPoint.x = map(touchPoint.x, TOUCH_X_MIN, TOUCH_X_MAX, tft.width(), 0);
  touchPoint.y = map(touchPoint.y, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, tft.height());

  std::array<bool, 14> xNumpadElement = {false};

  xNumpadElement.at(0) = touchArea(touchPoint.x, touchPoint.y, 196, 163, 238, 232); // Number 0 on numpad
  xNumpadElement.at(12) = touchArea(touchPoint.x, touchPoint.y, 196, 83, 238, 153); // komma on numpad
  xNumpadElement.at(13) = touchArea(touchPoint.x, touchPoint.y, 196, 244, 237, 316); // Abort on numpad

  xNumpadElement.at(1) = touchArea(touchPoint.x, touchPoint.y, 56, 244, 93, 316); // Number 1 on numpad
  xNumpadElement.at(2) = touchArea(touchPoint.x, touchPoint.y, 56, 163, 93, 233); // Number 2 on numpad
  xNumpadElement.at(3) = touchArea(touchPoint.x, touchPoint.y, 56, 83, 93, 153); // Number 3 on numpad
  xNumpadElement.at(10) = touchArea(touchPoint.x, touchPoint.y, 56, 1, 140, 72); // Enter on numpad

  xNumpadElement.at(4) = touchArea(touchPoint.x, touchPoint.y, 101, 244, 140, 316); // Number 4 on numpad
  xNumpadElement.at(5) = touchArea(touchPoint.x, touchPoint.y, 101, 163, 140, 233); // Number 5 on numpad
  xNumpadElement.at(6) = touchArea(touchPoint.x, touchPoint.y, 101, 83, 140, 153); // Number 6 on numpad

  xNumpadElement.at(7) = touchArea(touchPoint.x, touchPoint.y, 146, 244, 185, 316); // Number 7 on numpad
  xNumpadElement.at(8) = touchArea(touchPoint.x, touchPoint.y, 146, 163, 185, 233); // Number 8 on numpad
  xNumpadElement.at(9) = touchArea(touchPoint.x, touchPoint.y, 146, 83, 185, 153); // Number 9 on numpad
  xNumpadElement.at(11) = touchArea(touchPoint.x, touchPoint.y, 146, 1, 238, 72); // CLR on numpad

  for (auto i=0; i < xNumpadElement.size(); i++)
  {
    if (xNumpadElement.at(i))
    {
      return i; 
    }
  }
}

auto numPadHandler() -> void
{
  static bool xFinished = false; // true if enter was pressed
  static String sTempValue = ""; // temporary value for editing

  // sVariableName 
  // get numpad insert
  byte byNumpadInput = numpadTouch();
  switch (byNumpadInput)
  {
    case 0: // Number 0 was pressed
    sTempValue += "0";
    break; 
    case 1: // Number 1 was pressed
    sTempValue += "1";
    break;
    case 2: // Number 2 was pressed
    sTempValue += "2";
    break;
    case 3: // Number 3 was pressed
    sTempValue += "3";
    break;
    case 4: // Number 4 was pressed
    sTempValue += "4";
    break;
    case 5: // Number 5 was pressed
    sTempValue += "5";
    break;
    case 6: // Number 6 was pressed
    sTempValue += "6";
    break;
    case 7: // Number 7 was pressed
    sTempValue += "7";
    break;
    case 8: // Number 8 was pressed
    sTempValue += "8";
    break;
    case 9: // Number 9 was pressed
    sTempValue += "9";
    break;
    case 10: // Enter was pressed
    xFinished = true;
    break;
    case 11: // CLR was pressed
    sTempValue = "";
    break;
    case 12: // komma was pressed
    sTempValue += ",";
    break;
    case 13: // Abort was pressed
    xFinished = true;
    sTempValue = "";
    break;
  }

  // map the temporary value to the variable
  if(xFinished)
  {
    if (sVariableName == "uiSett_PVswitchingDelay")
    {
      uiSett_PVswitchingDelay = sTempValue.toInt();
    }
    if(sVariableName == "fTargetVoltageInverter")
    {
      fSett_TargetVoltageInverter = sTempValue.toFloat();
    }
    if(sVariableName == "fTargetVoltageMppt")
    {
      fSett_TargetVoltageMppt = sTempValue.toFloat();
    }
    if(sVariableName == "fSwitchCurrentTarget")
    {
      fSett_SwitchCurrentTarget = sTempValue.toFloat();
    }

    sVariableName = "";
    xFinished = false;
    byPageId = byPageIdAfterNumpad;
    byPageIdAfterNumpad = 0;
  }

}