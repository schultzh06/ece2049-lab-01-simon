/**************************************************************************
  This is a library for several Adafruit displays based on ST77* drivers.

  Works with the Adafruit 1.8" TFT Breakout w/SD card
    ----> http://www.adafruit.com/products/358
  The 1.8" TFT shield
    ----> https://www.adafruit.com/product/802
  The 1.44" TFT breakout
    ----> https://www.adafruit.com/product/2088
  as well as Adafruit raw 1.8" TFT display
    ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams.
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional).

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries. Adapted for STM32 for ECE 2049 by Kyle Schmottlach and Shannon Miranda '26
  MIT license, all text above must be included in any redistribution
 **************************************************************************/

#ifndef _ADAFRUIT_ST77XXH_
#define _ADAFRUIT_ST77XXH_

// #include "Arduino.h"
// #include "Print.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include "Adafruit_ST77xx_Macros.h"

#define boolean bool // Get around arduino using boolean instead of bool for some reason

/// Subclass of SPITFT for ST77xx displays (lots in common!)
class Adafruit_ST77xx : public Adafruit_SPITFT {
public:
  Adafruit_ST77xx(uint16_t w, uint16_t h, SPI_HandleTypeDef *spiHandle, int16_t cs, GPIO_TypeDef *cs_port,
                  int16_t dc, GPIO_TypeDef *dc_port, int8_t RST = -1, volatile bool *spiTxDone = nullptr);

  void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
  void setRotation(uint8_t r);
  void enableDisplay(boolean enable);
  void enableTearing(boolean enable);
  void enableSleep(boolean enable);

protected:
  uint8_t _colstart = 0,   ///< Some displays need this changed to offset
      _rowstart = 0;       ///< Some displays need this changed to offset
      // spiMode = SPI_MODE0; ///< Certain display needs MODE3 instead

  void begin(uint32_t freq = 0);
  void commonInit(const uint8_t *cmdList);
  void displayInit(const uint8_t *addr);
  void setColRowStart(int8_t col, int8_t row);
};

#endif // _ADAFRUIT_ST77XXH_
