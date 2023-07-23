// ##################
// LYQUID CRYSTAL SPI CODE
// ##################
void lcdInit() {
  Serial.println("Start Init ILI9341");
  tft.begin();
  delay(1000);
  touch.begin();
  delay(1000);
  if(!touch.begin()){
    Serial.println("Could not start touch controller xpt2046");
  }
  tft.setRotation(3);
  // TODO: make brightness changeable
  // TODO: draw screen in the background and after finishing turn on tft backlight
  analogWrite(TFT_LED,75); 
}
// TODO: turn off background if no touch input
void lcdStandbyHandler() {

}

// #######
// ILI9341: Drawing linear gradient
// #######
void drawGradientBackground() {
  int numPixels = tft.width();

  // Farbverlauf erstellen
  for (int x = 0; x < numPixels; x++) {
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

// Show Graphics on mainscreen;
// commit the byModeActual and return the updated operating mode;
byte showMainscreen(byte byMode, byte byModeOld) {
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_BLACK);
  byPageId = PAGE_MAIN;

  if (byMode == BATTERY_MODE && byModeOld != BATTERY_MODE) {
    drawGradientBackground();
    drawImage(cSolarPanelOn, SYMBOL_HEIGHT, SYMBOL_WIDTH, 30, 150);
    drawImage(cBatteryOn, SYMBOL_HEIGHT, SYMBOL_WIDTH, 127, 150);
    drawConnectionLine(byMode);
    drawImage(cValueBox, VALUEBOX_HEIGHT, VALUEBOX_WIDTH, 59, 55);
    drawImage(cInverterOff, SYMBOL_HEIGHT, SYMBOL_WIDTH, 224, 150);
    return  BATTERY_MODE;
  } else if (byMode == INVERTER_MODE && byModeOld != INVERTER_MODE) {
    drawGradientBackground();
    drawImage(cSolarPanelOn, SYMBOL_HEIGHT, SYMBOL_WIDTH, 30, 150);
    drawImage(cInverterOn, SYMBOL_HEIGHT, SYMBOL_WIDTH, 224, 150);
    drawConnectionLine(byMode);
    drawImage(cValueBox, VALUEBOX_HEIGHT, VALUEBOX_WIDTH, 107, 55);
    drawImage(cBatteryOff, SYMBOL_HEIGHT, SYMBOL_WIDTH, 127, 150);
    return  INVERTER_MODE;
  }
}

// UPDATING Arduino Variables on Display each second
// TODO: Create area which can be reloaded again
void showValues(byte byPage, byte byMode) {
  if (byPage == PAGE_MAIN) {
    if (byMode == BATTERY_MODE) {
      //"delete" old text
      tft.fillRect(65, 60, 95, 30, ILI9341_WHITE); //ftf.fillRect(xStart,yStart,xEnd,yEnd,color);
      tft.setCursor(70, 68); // tft.setCursor(x,y);
      tft.print(String(fCurrMppt));
      tft.print(" A");
    } else if (byMode == INVERTER_MODE) {
      tft.fillRect(112, 60, 95, 30, ILI9341_WHITE);
      tft.setCursor(118, 68);
      tft.print(String(fCurrInverter));
      tft.print(" A");
    }
  }
}

// #######
// ILI9341: Draw shape: inverted U for connectoion line on mainscreen
// #######
//Syntax: tft.drawLine(xStart,yStart,xEnd,yEnd,Color);
void drawConnectionLine(byte byMode) {
  //Connection line between SolarPanel and Battery
  if (byMode == BATTERY_MODE) {
    //vertical line Solar Panel, width: 3px
    tft.drawLine(62, 150, 62, 100, ILI9341_DARKGREEN);
    tft.drawLine(63, 150, 63, 100, ILI9341_DARKGREEN);
    tft.drawLine(64, 150, 64, 100, ILI9341_DARKGREEN);
    //horizontal line, width: 3px with 1px corner
    tft.drawLine(62, 100, 161, 100, ILI9341_DARKGREEN);
    tft.drawLine(63, 99, 160, 99, ILI9341_DARKGREEN);
    tft.drawLine(64, 98, 159, 98, ILI9341_DARKGREEN);
    //vertical line battery, width: 3px
    tft.drawLine(159, 150, 159, 100, ILI9341_DARKGREEN);
    tft.drawLine(160, 150, 160, 100, ILI9341_DARKGREEN);
    tft.drawLine(161, 150, 161, 100, ILI9341_DARKGREEN);
  } else if (byMode == INVERTER_MODE) {
    //vertical line Solar Panel, width: 3px
    tft.drawLine(62, 150, 62, 100, ILI9341_DARKGREEN);
    tft.drawLine(63, 150, 63, 100, ILI9341_DARKGREEN);
    tft.drawLine(64, 150, 64, 100, ILI9341_DARKGREEN);
    //horizontal line, width: 3px with 1px corner
    tft.drawLine(62, 100, 261, 100, ILI9341_DARKGREEN);
    tft.drawLine(63, 99, 260, 99, ILI9341_DARKGREEN);
    tft.drawLine(64, 98, 259, 98, ILI9341_DARKGREEN);
    //vertical line battery, width: 3px
    tft.drawLine(259, 150, 259, 100, ILI9341_DARKGREEN);
    tft.drawLine(260, 150, 260, 100, ILI9341_DARKGREEN);
    tft.drawLine(261, 150, 261, 100, ILI9341_DARKGREEN);
  }
}

// #######
// ILI9341: CONVERT char Array to Image on Screen
// #######
// Function for drawing the img
void drawImage(const char* cImgArray, byte byHeight, byte byWidth, byte byX, byte byY) {
  // Transmit picture data onto screen  for each line
  for (int y = 0; y < byHeight; y++) {
    for (int x = 0; x < byWidth; x++) {
      int index = y * byWidth + x;
      uint8_t pixel = pgm_read_byte(&cImgArray[index]);
      uint16_t color = convertPixelToColor(pixel);
      tft.drawPixel(byX + x, byY + y, color);
    }
  }
}
// Converting 8 Bit into 16 bit color
uint16_t convertPixelToColor(uint8_t pixel) {
  uint8_t r = (pixel & 0xE0) >> 5; // Rote Komponente (RRR00000)
  uint8_t g = (pixel & 0x1C) >> 2; // Grüne Komponente (GGG00)
  uint8_t b = (pixel & 0x03);      // Blaue Komponente (BB)
  r = (r << 5) | (r << 2) | (r >> 1); // Auf 8 Bit skalieren (RRRRR000)
  g = (g << 5) | (g << 2) | (g >> 1); // Auf 8 Bit skalieren (GGGGG000)
  b = (b << 6) | (b << 4) | (b << 2) | b; // Auf 8 Bit skalieren (BBBBB)
  return tft.color565(r, g, b); // 16-Bit-Farbwert zurückgeben
}