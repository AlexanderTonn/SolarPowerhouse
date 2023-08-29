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
  TS_Point touchPoint = touch.getPoint();
  touchPoint.x = map(touchPoint.x, TOUCH_X_MIN, TOUCH_X_MAX, tft.width(), 0);
  touchPoint.y = map(touchPoint.y, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, tft.height());
  Serial.print("(x: ");
  Serial.print(touchPoint.x);
  Serial.print(")");
  Serial.print("(y: ");
  Serial.print(touchPoint.y);
  Serial.println(")");
  Serial.print("Touch Pressure: ");
  Serial.println(touchPoint.z);

  // Display HMI Pages
  detectTouch(touchPoint.x, touchPoint.y, byPageId);

  bool x1 = functionTrigger(uiTimePageElasped, uiPageRefreshTime);

  if (x1)
  {
    if (byPageId == PAGE_MAIN)
    {
      showMainscreen(byModeActual, byModeOld);
      showMenuBar(byPageId);
    }
    else if (byPageId == PAGE_SETTINGS && byPageId != byPageIdOld)
    {
      byModeOld = 0; // delete. current "old" mode for reloading if returning to mainscreen
      showSettingsMenu();
    }
    else if (byPageId == PAGE_SETT_PV_SWITCHING && byPageId != byPageIdOld)
    {
      showSett_PvPageSwitching();
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
  if (byPage == PAGE_MAIN)
  {
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_BLACK);
    if (byOption == BATTERY_MODE)
    {
      updateValue(fCurrMppt, "A", 7, 2, ILI9341_BLACK, 70, 68);
    }
    else if (byOption == INVERTER_MODE)
    {
      updateValue(fCurrInverter,"A",7,2,ILI9341_BLACK,118,68);
    }
    updateValue(fBatteryVoltage,"V",7,1,ILI9341_BLACK,140,220);
    updateValue(fSolarVoltage, "V",7,1,ILI9341_BLACK,44,220);
  }else if(byPage == PAGE_SETT_PV_SWITCHING)
  {
    updateValue(uiSett_PVswitchingDelay,"s",5,2,ILI9341_BLACK, 235, 154);
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

// detect touch inputs
auto detectTouch(uint16_t x, uint16_t y, byte &byPageId) -> void
{

  if (x == 0 && y == 0)
  {
    return;
  }
  else
  {
    
    bool xT1 = touchArea(x, y , 256 , 192 , 291 , 224);     // Settings btn:
    bool xT2 = touchArea(x,y,46,88,282,117);                // Settings: Control Logic btn
    if (byPageId == PAGE_MAIN && xT1)
    {
      byPageId = PAGE_SETTINGS;
    }
    else if (byPageId == PAGE_SETTINGS && xT2)
    {
      byPageId = PAGE_SETT_PV_SWITCHING;
    }
    // Settings: PV-Switching Page
    else if(byPageId == PAGE_SETT_PV_SWITCHING)
    {
      bool xT1 = touchArea(x,y,289,175,305,191);                 // PV on Inverter
      bool xT2 = touchArea(x,y,287,141,308,163);                // PV on Mppt
      bool xT3 = touchArea(x,y,288,108,306,128);               // Reset Day

      if(xT1)
      {
        xSett_PVonInverter = changeCheckbox(xSett_PVonInverter);
        showCheckboxState(xSett_PVonInverter, 280, 40);
      }else if(xT2)
      {
        xSett_PVonMppt = changeCheckbox(xSett_PVonMppt);
        showCheckboxState(xSett_PVonMppt, 280, 75);
      }else if(xT3)
      {
        xSett_ResetDay = changeCheckbox( xSett_ResetDay);
        showCheckboxState(xSett_ResetDay, 280, 110);
      }
       delay(100);   // preventing bouncing, probably you have later a better solution
    }
   
      // Home Btn pressed
    if (byPageId > 1 && x >= 147 && x <= 183 && y >= 19 && y <= 46)
    {
      byPageId = PAGE_MAIN;
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
template<typename T>
auto updateValue(T T_variable, String sUnit, uint16_t uiLength, uint8_t uiTextSize, uint16_t uiColor, uint16_t uiX, uint16_t uiY) -> void
{
  uint8_t uiHeight = 0;
  uint16_t uiX_start = uiX-1, uiY_start = uiY-1, uiX_end = 0; 

  if(uiTextSize == 1)
  {
    uiX_end = uiLength * 6;
    uiHeight = 10;
  }
  else if(uiTextSize == 2)
  {
    uiX_end = uiLength * uiTextSize * 6;
    uiHeight = 20; 

  }else
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
auto showCheckboxState(bool xVariable, uint16_t uiX, uint16_t uiY) -> void
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
  if(xCurrentState)
  {
    return xCurrentState = false;
  }else
  {
    return xCurrentState = true;
  }
}

auto touchArea(int16_t x, int16_t y ,uint16_t xStart,uint16_t yStart,uint16_t xEnd,uint16_t yEnd) -> bool {
  if(x >= xStart && x <= xEnd && y >= yStart && y <= yEnd)
  {
    return true;
  }else
  {
    return false;
  }
}
