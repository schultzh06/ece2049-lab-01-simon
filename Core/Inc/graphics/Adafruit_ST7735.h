/*
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
MIT license, all text above must be included in any redistribution.
*/

#ifndef _ADAFRUIT_ST7735H_
#define _ADAFRUIT_ST7735H_

#include "Adafruit_ST77xx.h"
#include "Adafruit_ST7735_Macros.h"

/// Subclass of ST77XX for ST7735B and ST7735R TFT Drivers:
class Adafruit_ST7735 : public Adafruit_ST77xx {
public:
  Adafruit_ST7735(SPI_HandleTypeDef *spiHandle, int16_t cs, GPIO_TypeDef *cs_port, int16_t dc, GPIO_TypeDef *dc_port, int8_t rst, volatile bool *spiTxDone = nullptr);

  // Differences between displays (usu. identified by colored tab on
  // plastic overlay) are odd enough that we need to do this 'by hand':
  void initB(void);                             // for ST7735B displays
  void initR(uint8_t options = INITR_GREENTAB); // for ST7735R

  void setRotation(uint8_t m);

private:
  uint8_t tabcolor;
};

#endif // _ADAFRUIT_ST7735H_
