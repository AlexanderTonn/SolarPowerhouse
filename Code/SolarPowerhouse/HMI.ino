#include "HMI.h"

// ##################
// LYQUID CRYSTAL SPI CODE
// ##################
auto lcdInit() -> void
{
  Serial.println("Start Init ILI9341");
  tft.begin();
  delay(1000);
  touch.begin();
  delay(1000);
  if (!touch.begin())
  {
    Serial.println("Could not start touch controller xpt2046");
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
  uint16_t uiX = touchPoint.x;
  uint16_t uiY = touchPoint.y;
  Serial.print("(x: ");
  Serial.print(uiX);
  Serial.print(")");
  Serial.print("(y: ");
  Serial.print(uiY);
  Serial.println(")");

  // Display HMI Pages
  detectTouch(uiX, uiY, byPageId);

  bool x1 = functionTrigger(uiTimePageElasped, uiPageRefreshTime);
  Serial.println(x1);

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
    else if (byPageId == PAGE_CTRL_LOGIC_1 && byPageId != byPageIdOld)
    {
      showCtrlLogicSettings();
    }
    if (byPageId != byPageIdOld)
    {
      byPageIdOld = byPageId;
    }
  }

  bool x2 = functionTrigger(uiTimeValueElapsed, uiValueRefreshTime);
  Serial.println(x2);
  if (x2 && byPageId == PAGE_MAIN)
  {
    showValues(PAGE_MAIN, byModeActual);
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
auto showValues(byte byPage, byte byMode) -> void
{
  if (byPage == PAGE_MAIN)
  {
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_BLACK);
    if (byMode == BATTERY_MODE)
    {
      //"delete" old text
      tft.fillRect(65, 60, 95, 30, ILI9341_WHITE); // ftf.fillRect(xStart,yStart,xEnd,yEnd,color);
      tft.setCursor(70, 68);                       // tft.setCursor(x,y);
      tft.print(String(fCurrMppt));
      tft.print(" A");
    }
    else if (byMode == INVERTER_MODE)
    {
      tft.fillRect(112, 60, 95, 30, ILI9341_WHITE);
      tft.setCursor(118, 68);
      tft.print(String(fCurrInverter));
      tft.print(" A");
    }
    tft.setTextSize(1);
    tft.fillRect(40, 215, 50, 20, ILI9341_WHITE);
    tft.setCursor(44, 220);
    tft.print(String(fSolarVoltage) + " V");
    tft.fillRect(140, 215, 50, 20, ILI9341_WHITE);
    tft.setCursor(140, 220);
    tft.print(String(fBatteryVoltage) + " V");
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
    // Settings btn:
    if (x >= 4 && x <= 41 && y >= 29 && y <= 69)
    {
      byPageId = PAGE_SETTINGS;
    }
    // Settings: Control Logic btn
    if (byPageId == PAGE_SETTINGS && x >= 125 && x <= 153 && y >= 34 && y <= 280)
    {
      byPageId = PAGE_CTRL_LOGIC_1;
    }
    // Home Btn pressed
    else if (byPageId > 1 && x >= 190 && x <= 226 && y >= 135 && y <= 176)
    {
      byPageId = PAGE_MAIN;
    }
  }
}