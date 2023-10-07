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

  if (x1)
  {
    if (byPageId == MAIN)
    {
      showMainscreen();
      showMenuBar(byPageId);
    }
    else
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
                       true, true);
    }
    else if (byPageId == SETT_PV_SWITCHING2 && byPageId != byPageIdOld)
    {
      drawSettingsPage("Schalt-Grenzwerte",
                       "Strom ignorieren", guiElement::CHECKBOX, xSett_ignoreCurrent,
                       "Inverter, ab >", guiElement::TEXT_INPUT, fSett_TargetVoltageInverter,
                       "Batterie, ab <", guiElement::TEXT_INPUT, fSett_TargetVoltageMppt,
                       "Schalten, bei < ", guiElement::TEXT_INPUT, fSett_SwitchCurrentTarget,
                       true, false);
    }
    else if (byPageId == NUMPAD && byPageId != byPageIdOld)
    {
      drawNumpad(sVariableName);
    }

    if (byPageId != byPageIdOld)
    {
      byPageIdOld = byPageId;
    }
  }

  // handle numpad
  if (!byPageIdAfterNumpad == 0)
  {
    numPadHandler();
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
  if (xStandby)
  {
    for (auto i = uiTftBrightness; i > 0; i--)
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
  switch (byPage)
  {
  case MAIN:
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_BLACK);
    if (!xSett_ignoreCurrent)
    {
      if (byOption == BATTERY_MODE)
      {
        updateValue(fCurrMppt, "A", 7, 2, ILI9341_BLACK, 70, 68);
      }
      else if (byOption == INVERTER_MODE)
      {
        updateValue(fCurrInverter, "A", 7, 2, ILI9341_BLACK, 118, 68);
      }
    }

    updateValue(fBatteryVoltage, "V", 7, 1, ILI9341_BLACK, 140, 220);
    updateValue(fSolarVoltage, "V", 7, 1, ILI9341_BLACK, 44, 220);
    break;

  case SETT_PV_SWITCHING1:
    updateValue(uiSett_PVswitchingDelay, "s", 5, 2, ILI9341_BLACK, 235, 154);
    break;
  case SETT_PV_SWITCHING2:
    updateValue(fSett_TargetVoltageInverter, "V", 5, 2, ILI9341_BLACK, 225, 85);
    updateValue(fSett_TargetVoltageMppt, "V", 5, 2, ILI9341_BLACK, 225, 120);
    updateValue(fSett_SwitchCurrentTarget, "A", 5, 2, ILI9341_BLACK, 225, 155);
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
  if (touch.touched())
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

    bool xSettingsTouched = touchArea(x, y, 256, 192, 291, 224);  // Settings btn:
    bool xConrolLogicTouched = touchArea(x, y, 46, 88, 282, 117); // Settings: Control Logic btn
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
      byte byGuiElement = touchSettingsPage(x, y,
                                            guiElement::CHECKBOX,
                                            guiElement::CHECKBOX,
                                            guiElement::CHECKBOX,
                                            guiElement::TEXT_INPUT,
                                            true);
      if (byGuiElement == 1)
      {
        xSett_PVonMppt = changeCheckbox(xSett_PVonMppt);
        xSett_PVonInverter = false; // if PV on Mppt is selected, PV on Inverter is not allowed
        showCheckboxState(xSett_PVonMppt, 280, 40);
        showCheckboxState(xSett_PVonInverter, 280, 75);
      }
      else if (byGuiElement == 2)
      {
        xSett_PVonInverter = changeCheckbox(xSett_PVonInverter);
        xSett_PVonMppt = false; // if PV on Inverter is selected, PV on Mppt is not allowed
        showCheckboxState(xSett_PVonMppt, 280, 40);
        showCheckboxState(xSett_PVonInverter, 280, 75);
      }
      else if (byGuiElement == 3)
      {
        xSett_ResetDay = changeCheckbox(xSett_ResetDay);
        showCheckboxState(xSett_ResetDay, 280, 110);
      }
      else if (byGuiElement == 4)
      {
        byPageId = NUMPAD;
        byPageIdAfterNumpad = SETT_PV_SWITCHING1;
        sVariableName = "Schaltabstand [s]";
      }
      delay(100); // preventing bouncing, probably you have later a better solution
    }
    else if (byPageId == SETT_PV_SWITCHING2)
    {
      byte byGuiElement = touchSettingsPage(x, y,
                                            guiElement::CHECKBOX,
                                            guiElement::TEXT_INPUT,
                                            guiElement::TEXT_INPUT,
                                            guiElement::TEXT_INPUT,
                                            true);
      if (byGuiElement == 1)
      {
        xSett_ignoreCurrent = changeCheckbox(xSett_ignoreCurrent);
        showCheckboxState(xSett_ignoreCurrent, 280, 40);
      }
      else if (byGuiElement == 2)
      {
        byPageId = NUMPAD;
        byPageIdAfterNumpad = SETT_PV_SWITCHING2;
        sVariableName = "Inverter, ab > [V]";
      }
      else if (byGuiElement == 3)
      {
        byPageId = NUMPAD;
        byPageIdAfterNumpad = SETT_PV_SWITCHING2;
        sVariableName = "Batterie, ab < [V]";
      }
      else if (byGuiElement == 4)
      {
        byPageId = NUMPAD;
        byPageIdAfterNumpad = SETT_PV_SWITCHING2;
        sVariableName = "Schalten, bei < [A]";
      }
    }

    // Home Btn pressed
    if (byPageId > 1 && byPageId != NUMPAD && touchArea(x, y, 147, 19, 183, 46))
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

  // preventing bouncing, probably you have later a better solution

  if (touch.touched())
  {
    TS_Point touchPoint = touch.getPoint();
    touchPoint.x = map(touchPoint.x, TOUCH_X_MIN, TOUCH_X_MAX, tft.width(), 0);
    touchPoint.y = map(touchPoint.y, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, tft.height());

    std::array<bool, 14> xNumpadElement = {false};
    
    xNumpadElement.at(0) = touchArea(touchPoint.x, touchPoint.y, 90, 4, 153, 38);    // Number 0 on numpad
    xNumpadElement.at(12) = touchArea(touchPoint.x, touchPoint.y, 170, 4, 233, 38);  // komma on numpad
    xNumpadElement.at(13) = touchArea(touchPoint.x, touchPoint.y, 10, 4, 75, 38); // Abort on numpad

    xNumpadElement.at(1) = touchArea(touchPoint.x, touchPoint.y, 8, 146, 77,  180); // Number 1 on numpad
    xNumpadElement.at(2) = touchArea(touchPoint.x, touchPoint.y, 90, 146, 157,  180); // Number 2 on numpad
    xNumpadElement.at(3) = touchArea(touchPoint.x, touchPoint.y, 168, 146, 231,  180);  // Number 3 on numpad
    xNumpadElement.at(10) = touchArea(touchPoint.x, touchPoint.y, 250, 102, 310,  180);  // Enter on numpad

    xNumpadElement.at(4) = touchArea(touchPoint.x, touchPoint.y, 8, 102, 77, 133); // Number 4 on numpad
    xNumpadElement.at(5) = touchArea(touchPoint.x, touchPoint.y, 90, 102, 157, 133); // Number 5 on numpad
    xNumpadElement.at(6) = touchArea(touchPoint.x, touchPoint.y, 168, 102, 231, 133);  // Number 6 on numpad

    xNumpadElement.at(7) = touchArea(touchPoint.x, touchPoint.y, 8, 53, 77, 88); // Number 7 on numpad
    xNumpadElement.at(8) = touchArea(touchPoint.x, touchPoint.y, 90, 50, 157, 88); // Number 8 on numpad
    xNumpadElement.at(9) = touchArea(touchPoint.x, touchPoint.y, 168, 50, 231, 88);  // Number 9 on numpad
    xNumpadElement.at(11) = touchArea(touchPoint.x, touchPoint.y, 250, 8, 307, 90);   // CLR on numpad

    delay(200); // preventing bouncing, probably you have later a better solution

    for (auto i = 0; i < xNumpadElement.size(); i++)
    {
      if (xNumpadElement.at(i))
      {
        return i;
      }
    }
  
    
  }else
  return 255; // return 255 if no touch input
}

auto numPadHandler() -> void
{
  static bool xFinished = false; // true if enter was pressed
  bool xCleared = false, xAbort = false;         // true if CLR was pressed
  static String sTempValue = ""; // temporary value for editing
  static String sTempValueOld = ""; // temporary value for editing

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
    sTempValue = "     ";
    xCleared = true;
    break;
  case 12: // komma was pressed
    sTempValue += ".";
    break;
  case 13: // Abort was pressed
    sTempValue = "";
    xAbort = true;
    break;
  }

  if(sTempValue != sTempValueOld && sTempValue.length() < 6){
    sTempValueOld = sTempValue;
    updateValue(sTempValue, "", sTempValue.length(), 2, ILI9341_BLACK, 190, 20);
  }

  if(xCleared)
    sTempValue = "";

  if(xAbort)
  {
    sTempValue = "";
    byPageId = byPageIdAfterNumpad;
    byPageIdAfterNumpad = 0;
  }

  // map the temporary value to the variable
  if (xFinished)
  {
    if (sVariableName == "Schaltabstand [s]")
    {
      uiSett_PVswitchingDelay = sTempValue.toInt();
    }
    if (sVariableName == "Inverter, ab > [V]")
    {
      fSett_TargetVoltageInverter = sTempValue.toFloat();
    }
    if (sVariableName == "Batterie, ab < [V]")
    {
      fSett_TargetVoltageMppt = sTempValue.toFloat();
    }
    if (sVariableName == "Schalten, bei < [A]")
    {
      fSett_SwitchCurrentTarget = sTempValue.toFloat();
    }
    sVariableName = "";
    xFinished = false;
    byPageId = byPageIdAfterNumpad;
    byPageIdAfterNumpad = 0;
  }
}

// Detectin Touch on Settings Pages and return a byte value which is indicating the selected element
auto touchSettingsPage(uint16_t x, uint16_t y,
                       guiElement element1,
                       guiElement element2,
                       guiElement element3,
                       guiElement element4,
                       bool xSwitchPage) -> byte
{

  switch (element1)
  {
  case guiElement::CHECKBOX:
    if (x >= 288 && x <= 305 && y >= 175 && y <= 191)
      return 1;
    break;
  // TODO!: Check whether Text Input have correct coordinates
  case guiElement::TEXT_INPUT:
    if (x >= 213 && x <= 312 && y >= 40 && y <= 72)
      return 1;
    break;

  default:
    break;
  }
  switch (element2)
  {
  case guiElement::CHECKBOX:
    if (x >= 288 && x <= 305 && y >= 141 && y <= 163)
      return 2;
    break;
  case guiElement::TEXT_INPUT:
    if (x >= 219 && x <= 305 && y >= 139 && y <= 158)
      return 2;
    break;
  default:
    break;
  }
  switch (element3)
  {
  case guiElement::CHECKBOX:
    if (x >= 288 && x <= 305 && y >= 108 && y <= 128)
      return 3;
    break;
  case guiElement::TEXT_INPUT:
    if (x >= 218 && x <= 302 && y >= 102 && y <= 122)
      return 3;
    break;
  default:
    break;
  }
  switch (element4)
  {
  case guiElement::CHECKBOX:
    if (x >= 288 && x <= 305 && y >= 145 && y <= 161)
      return 4;
    break;
  case guiElement::TEXT_INPUT:
    if (x >= 222 && x <= 312 && y >= 70 && y <= 88)
      return 4;
    break;
  }
  if (xSwitchPage)
  {
    // Previous Page
    if (x >= 94 && x <= 125 && y >= 16 && y <= 42)
      byPageId -= 1;
    // Next Page
    if (x >= 199 && x <= 230 && y >= 16 && y <= 42 && byPageId < NUMPAD - 1)
      byPageId += 1;

    return 0;
  }
}