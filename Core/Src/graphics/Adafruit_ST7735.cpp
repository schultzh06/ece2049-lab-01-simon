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

#include "Adafruit_ST7735.h"
#include "Adafruit_ST77xx.h"

// CONSTRUCTORS ************************************************************

/*!
    @brief  Instantiate Adafruit ST7735 driver with selectable hardware SPI
    @param  spiClass  Pointer to an SPI device to use (e.g. &SPI1)
    @param  cs        Chip select pin #
    @param  dc        Data/Command pin #
    @param  rst       Reset pin # (optional, pass -1 if unused)
*/
Adafruit_ST7735::Adafruit_ST7735(SPI_HandleTypeDef *spiHandle, int16_t cs, GPIO_TypeDef *cs_port, int16_t dc, GPIO_TypeDef *dc_port, int8_t rst, volatile bool *spiTxDone)
    : Adafruit_ST77xx(ST7735_TFTWIDTH_128, ST7735_TFTHEIGHT_160, spiHandle, cs, cs_port, dc, dc_port, rst, spiTxDone) {}

// SCREEN INITIALIZATION ***************************************************

// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.

// clang-format off
static constexpr uint8_t 
  Rcmd1[] = {                       // 7735R init, part 1 (red or green tab)
    15,                             // 15 commands in list:
    ST77XX_SWRESET,   ST_CMD_DELAY, //  1: Software reset, 0 args, w/delay
      150,                          //     150 ms delay
    ST77XX_SLPOUT,    ST_CMD_DELAY, //  2: Out of sleep mode, 0 args, w/delay
      255,                          //     500 ms delay
    ST7735_FRMCTR1, 3,              //  3: Framerate ctrl - normal mode, 3 arg:
      0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2, 3,              //  4: Framerate ctrl - idle mode, 3 args:
      0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3, 6,              //  5: Framerate - partial mode, 6 args:
      0x01, 0x2C, 0x2D,             //     Dot inversion mode
      0x01, 0x2C, 0x2D,             //     Line inversion mode
    ST7735_INVCTR,  1,              //  6: Display inversion ctrl, 1 arg:
      0x07,                         //     No inversion
    ST7735_PWCTR1,  3,              //  7: Power control, 3 args, no delay:
      0xA2,
      0x02,                         //     -4.6V
      0x84,                         //     AUTO mode
    ST7735_PWCTR2,  1,              //  8: Power control, 1 arg, no delay:
      0xC5,                         //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
    ST7735_PWCTR3,  2,              //  9: Power control, 2 args, no delay:
      0x0A,                         //     Opamp current small
      0x00,                         //     Boost frequency
    ST7735_PWCTR4,  2,              // 10: Power control, 2 args, no delay:
      0x8A,                         //     BCLK/2,
      0x2A,                         //     opamp current small & medium low
    ST7735_PWCTR5,  2,              // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
    ST7735_VMCTR1,  1,              // 12: Power control, 1 arg, no delay:
      0x0E,
    ST77XX_INVOFF,  0,              // 13: Don't invert display, no args
    ST77XX_MADCTL,  1,              // 14: Mem access ctl (directions), 1 arg:
      0xC8,                         //     row/col addr, bottom-top refresh
    ST77XX_COLMOD,  1,              // 15: set color mode, 1 arg, no delay:
      0x05 },                       //     16-bit color

  Rcmd2green[] = {                  // 7735R init, part 2 (green tab only)
    2,                              //  2 commands in list:
    ST77XX_CASET,   4,              //  1: Column addr set, 4 args, no delay:
      0x00, 0x02,                   //     XSTART = 0
      0x00, 0x7F+0x02,              //     XEND = 127
    ST77XX_RASET,   4,              //  2: Row addr set, 4 args, no delay:
      0x00, 0x01,                   //     XSTART = 0
      0x00, 0x9F+0x01 },            //     XEND = 159

  Rcmd2red[] = {                    // 7735R init, part 2 (red tab only)
    2,                              //  2 commands in list:
    ST77XX_CASET,   4,              //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x7F,                   //     XEND = 127
    ST77XX_RASET,   4,              //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x9F },                 //     XEND = 159

  Rcmd2green144[] = {               // 7735R init, part 2 (green 1.44 tab)
    2,                              //  2 commands in list:
    ST77XX_CASET,   4,              //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x7F,                   //     XEND = 127
    ST77XX_RASET,   4,              //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x7F },                 //     XEND = 127

  Rcmd2green160x80[] = {            // 7735R init, part 2 (mini 160x80)
    2,                              //  2 commands in list:
    ST77XX_CASET,   4,              //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x4F,                   //     XEND = 79
    ST77XX_RASET,   4,              //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x9F },                 //     XEND = 159

  Rcmd2green160x80plugin[] = {      // 7735R init, part 2 (mini 160x80 with plugin FPC)
    3,                              //  3 commands in list:
    ST77XX_INVON,  0,              //   1: Display is inverted
    ST77XX_CASET,   4,              //  2: Column addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x4F,                   //     XEND = 79
    ST77XX_RASET,   4,              //  3: Row addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x9F },                 //     XEND = 159

  Rcmd3[] = {                       // 7735R init, part 3 (red or green tab)
    4,                              //  4 commands in list:
    ST7735_GMCTRP1, 16      ,       //  1: Gamma Adjustments (pos. polarity), 16 args + delay:
      0x02, 0x1c, 0x07, 0x12,       //     (Not entirely necessary, but provides
      0x37, 0x32, 0x29, 0x2d,       //      accurate colors)
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16      ,       //  2: Gamma Adjustments (neg. polarity), 16 args + delay:
      0x03, 0x1d, 0x07, 0x06,       //     (Not entirely necessary, but provides
      0x2E, 0x2C, 0x29, 0x2D,       //      accurate colors)
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST77XX_NORON,     ST_CMD_DELAY, //  3: Normal display on, no args, w/delay
      10,                           //     10 ms delay
    ST77XX_DISPON,    ST_CMD_DELAY, //  4: Main screen turn on, no args w/delay
      100 };                        //     100 ms delay

// clang-format on

/**************************************************************************/
/*!
    @brief  Initialization code common to all ST7735R displays
    @param  options  Tab color from adafruit purchase
*/
/**************************************************************************/
void Adafruit_ST7735::initR(uint8_t options) {
  printf("Running common init...\n\r");
  commonInit(Rcmd1);
  printf("Completed common init!\n\r");
  if (options == INITR_GREENTAB) {
    displayInit(Rcmd2green);
    _colstart = 2;
    _rowstart = 1;
  } else if ((options == INITR_144GREENTAB) || (options == INITR_HALLOWING)) {
    _height = ST7735_TFTHEIGHT_128;
    _width = ST7735_TFTWIDTH_128;
    displayInit(Rcmd2green144);
    _colstart = 2;
    _rowstart = 3; // For default rotation 0
  } else if (options == INITR_MINI160x80) {
    _height = ST7735_TFTWIDTH_80;
    _width = ST7735_TFTHEIGHT_160;
    displayInit(Rcmd2green160x80);
    _colstart = 24;
    _rowstart = 0;
  } else if (options == INITR_MINI160x80_PLUGIN) {
    _height = ST7735_TFTWIDTH_80;
    _width = ST7735_TFTHEIGHT_160;
    displayInit(Rcmd2green160x80plugin);
    _colstart = 26;
    _rowstart = 1;
    invertOnCommand = ST77XX_INVOFF;
    invertOffCommand = ST77XX_INVON;
  } else {
    // colstart, rowstart left at default '0' values
    displayInit(Rcmd2red);
  }
  displayInit(Rcmd3);

  // Black tab, change MADCTL color filter
  if ((options == INITR_BLACKTAB) || (options == INITR_MINI160x80)) {
    uint8_t data = 0xC0;
    sendCommand(ST77XX_MADCTL, &data, 1);
  }

  if (options == INITR_HALLOWING) {
    // Hallowing is simply a 1.44" green tab upside-down:
    tabcolor = INITR_144GREENTAB;
    setRotation(2);
  } else {
    tabcolor = options;
    setRotation(0);
  }
}

// OTHER FUNCTIONS *********************************************************

/**************************************************************************/
/*!
    @brief  Set origin of (0,0) and orientation of TFT display
    @param  m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Adafruit_ST7735::setRotation(uint8_t m) {
  uint8_t madctl = 0;

  rotation = m & 3; // can't be higher than 3

  // For ST7735 with GREEN TAB (including HalloWing)...
  if ((tabcolor == INITR_144GREENTAB) || (tabcolor == INITR_HALLOWING)) {
    // ..._rowstart is 3 for rotations 0&1, 1 for rotations 2&3
    _rowstart = (rotation < 2) ? 3 : 1;
  }

  switch (rotation) {
  case 0:
    if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
      madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB;
    } else {
      madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST7735_MADCTL_BGR;
    }

    if (tabcolor == INITR_144GREENTAB) {
      _height = ST7735_TFTHEIGHT_128;
      _width = ST7735_TFTWIDTH_128;
    } else if (tabcolor == INITR_MINI160x80 ||
               tabcolor == INITR_MINI160x80_PLUGIN) {
      _height = ST7735_TFTHEIGHT_160;
      _width = ST7735_TFTWIDTH_80;
    } else {
      _height = ST7735_TFTHEIGHT_160;
      _width = ST7735_TFTWIDTH_128;
    }
    _xstart = _colstart;
    _ystart = _rowstart;
    break;
  case 1:
    if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
      madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    } else {
      madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST7735_MADCTL_BGR;
    }

    if (tabcolor == INITR_144GREENTAB) {
      _width = ST7735_TFTHEIGHT_128;
      _height = ST7735_TFTWIDTH_128;
    } else if (tabcolor == INITR_MINI160x80 ||
               tabcolor == INITR_MINI160x80_PLUGIN) {
      _width = ST7735_TFTHEIGHT_160;
      _height = ST7735_TFTWIDTH_80;
    } else {
      _width = ST7735_TFTHEIGHT_160;
      _height = ST7735_TFTWIDTH_128;
    }
    _ystart = _colstart;
    _xstart = _rowstart;
    break;
  case 2:
    if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
      madctl = ST77XX_MADCTL_RGB;
    } else {
      madctl = ST7735_MADCTL_BGR;
    }

    if (tabcolor == INITR_144GREENTAB) {
      _height = ST7735_TFTHEIGHT_128;
      _width = ST7735_TFTWIDTH_128;
    } else if (tabcolor == INITR_MINI160x80 ||
               tabcolor == INITR_MINI160x80_PLUGIN) {
      _height = ST7735_TFTHEIGHT_160;
      _width = ST7735_TFTWIDTH_80;
    } else {
      _height = ST7735_TFTHEIGHT_160;
      _width = ST7735_TFTWIDTH_128;
    }
    _xstart = _colstart;
    _ystart = _rowstart;
    break;
  case 3:
    if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
      madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    } else {
      madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST7735_MADCTL_BGR;
    }

    if (tabcolor == INITR_144GREENTAB) {
      _width = ST7735_TFTHEIGHT_128;
      _height = ST7735_TFTWIDTH_128;
    } else if (tabcolor == INITR_MINI160x80 ||
               tabcolor == INITR_MINI160x80_PLUGIN) {
      _width = ST7735_TFTHEIGHT_160;
      _height = ST7735_TFTWIDTH_80;
    } else {
      _width = ST7735_TFTHEIGHT_160;
      _height = ST7735_TFTWIDTH_128;
    }
    _ystart = _colstart;
    _xstart = _rowstart;
    break;
  }

  sendCommand(ST77XX_MADCTL, &madctl, 1);
}
