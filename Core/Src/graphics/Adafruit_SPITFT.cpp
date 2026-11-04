/*!
 * @file Adafruit_SPITFT.cpp
 *
 * @mainpage Adafruit SPI TFT Displays (and some others)
 *
 * @section intro_sec Introduction
 *
 * Part of Adafruit's GFX graphics library. Originally this class was
 * written to handle a range of color TFT displays connected via SPI,
 * but over time this library and some display-specific subclasses have
 * mutated to include some color OLEDs as well as parallel-interfaced
 * displays. The name's been kept for the sake of older code.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!

 * @section dependencies Dependencies
 *
 * This library depends on
 * <a href="https://github.com/adafruit/Adafruit-GFX-Library">Adafruit_GFX</a>
 * being present on your system. Please make sure you have installed the latest
 * version before using this library.
 *
 * @section author Author
 *
 * Written by Limor "ladyada" Fried for Adafruit Industries,
 * with contributions from the open source community.
 *
 * Adapted for STM32 for ECE2049 by Kyle Schmottlach and Shannon Miranda '26
 *
 * @section license License
 *
 * BSD license, all text here must be included in any redistribution.
 */

#include "Adafruit_SPITFT.h"

// Possible values for Adafruit_SPITFT.connection:
#define TFT_HARD_SPI 0 ///< Display interface = hardware SPI
#define TFT_SOFT_SPI 1 ///< Display interface = software SPI
#define TFT_PARALLEL 2 ///< Display interface = 8- or 16-bit parallel

inline void flip_endianness(uint8_t *data, size_t length) {
  for (size_t i = 0; i < length / 2; i++) {
    uint8_t temp = data[i];
    data[i] = data[length - 1 - i];
    data[length - 1 - i] = temp;
  }
}

// CONSTRUCTORS ------------------------------------------------------------

/*!
    @brief   Adafruit_SPITFT constructor for hardware SPI using a specific
             SPI peripheral.
    @param   w         Display width in pixels at default rotation (0).
    @param   h         Display height in pixels at default rotation (0).
    @param   spiClass  Pointer to SPIClass type (e.g. &SPI or &SPI1).
    @param   cs        Arduino pin # for chip-select (-1 if unused, tie CS low).
    @param   dc        Arduino pin # for data/command select (required).
    @param   rst       Arduino pin # for display reset (optional, display reset
                       can be tied to MCU reset, default of -1 means unused).
    @note    Output pins are not initialized in constructor; application
             typically will need to call subclass' begin() function, which
             in turn calls this library's initSPI() function to initialize
             pins. EXCEPT...if you have built your own SERCOM SPI peripheral
             (calling the SPIClass constructor) rather than one of the
             built-in SPI devices (e.g. &SPI, &SPI1 and so forth), you will
             need to call the begin() function for your object as well as
             pinPeripheral() for the MOSI, MISO and SCK pins to configure
             GPIO manually. Do this BEFORE calling the display-specific
             begin or init function. Unfortunate but unavoidable.
*/
Adafruit_SPITFT::Adafruit_SPITFT(uint16_t w, uint16_t h, SPI_HandleTypeDef *spiHandle,
                                 int16_t cs_pin, GPIO_TypeDef *cs_port, int16_t dc, GPIO_TypeDef *dc_port, int8_t rst, volatile bool *spiTxDone)
    : Adafruit_GFX(w, h), connection(TFT_HARD_SPI), _rst(rst), _cs_pin(cs_pin), 
    _cs_port(cs_port), _dc_pin(dc), _dc_port(dc_port) {
  hwspi._spi = spiHandle;
  hwspi._spiTxDone = spiTxDone;
}

// end constructors -------

// CLASS MEMBER FUNCTIONS --------------------------------------------------

// begin() and setAddrWindow() MUST be declared by any subclass.

/*!
    @brief  Configure microcontroller pins for TFT interfacing. Typically
            called by a subclass' begin() function.
    @param  freq     SPI frequency when using hardware SPI. If default (0)
                     is passed, will fall back on a device-specific value.
                     Value is ignored when using software SPI or parallel
                     connection.
    @param  spiMode  SPI mode when using hardware SPI. MUST be one of the
                     values SPI_MODE0, SPI_MODE1, SPI_MODE2 or SPI_MODE3
                     defined in SPI.h. Do NOT attempt to pass '0' for
                     SPI_MODE0 and so forth...the values are NOT the same!
                     Use ONLY the defines! (Pity it's not an enum.)
    @note   Another anachronistically-named function; this is called even
            when the display connection is parallel (not SPI). Also, this
            could probably be made private...quite a few class functions
            were generously put in the public section.
*/
void Adafruit_SPITFT::initSPI(uint32_t freq) {

  if (!freq)
    freq = DEFAULT_SPI_FREQ; // If no freq specified, use default

  // Init basic control pins common to all connection types
  if (_cs_pin >= 0) {
    HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_SET); // Deselect
  }
  HAL_GPIO_WritePin(_dc_port, _dc_pin, GPIO_PIN_SET); // Data mode

  if (connection == TFT_HARD_SPI) {

#if defined(SPI_HAS_TRANSACTION)
    hwspi.settings = SPISettings(freq, MSBFIRST, spiMode);
#else
    hwspi._freq = freq; // Save freq value for later
#endif
    // Call hwspi._spi->begin() ONLY if this is among the 'established'
    // SPI interfaces in variant.h. For DIY roll-your-own SERCOM SPIs,
    // begin() and pinPeripheral() calls MUST be made in one's calling
    // code, BEFORE the screen-specific begin/init function is called.
    // Reason for this is that SPI::begin() makes its own calls to
    // pinPeripheral() based on g_APinDescription[n].ulPinType, which
    // on non-established SPI interface pins will always be PIO_DIGITAL
    // or similar, while we need PIO_SERCOM or PIO_SERCOM_ALT...it's
    // highly unique between devices and variants for each pin or
    // SERCOM so we can't make those calls ourselves here. And the SPI
    // device needs to be set up before calling this because it's
    // immediately followed with initialization commands. Blargh.
    
    // hwspi._spi->begin();
  }
}

/*!
    @brief  Allow changing the SPI clock speed after initialization
    @param  freq Desired frequency of SPI clock, may not be the
    end frequency you get based on what the chip can do!
*/
void Adafruit_SPITFT::setSPISpeed(uint32_t freq) {
#if defined(SPI_HAS_TRANSACTION)
  hwspi.settings = SPISettings(freq, MSBFIRST, hwspi._mode);
#else
  hwspi._freq = freq; // Save freq value for later
#endif
}

/*!
    @brief  Call before issuing command(s) or data to display. Performs
            chip-select (if required) and starts an SPI transaction (if
            using hardware SPI and transactions are supported). Required
            for all display types; not an SPI-specific function.
*/
void Adafruit_SPITFT::startWrite(void) {
  // SPI_BEGIN_TRANSACTION();
  if (_cs_pin >= 0)
    SPI_CS_LOW();
}

/*!
    @brief  Call after issuing command(s) or data to display. Performs
            chip-deselect (if required) and ends an SPI transaction (if
            using hardware SPI and transactions are supported). Required
            for all display types; not an SPI-specific function.
*/
void Adafruit_SPITFT::endWrite(void) {
  if (_cs_pin >= 0)
    SPI_CS_HIGH();
  // SPI_END_TRANSACTION();
}

// -------------------------------------------------------------------------
// Lower-level graphics operations. These functions require a chip-select
// and/or SPI transaction around them (via startWrite(), endWrite() above).
// Higher-level graphics primitives might start a single transaction and
// then make multiple calls to these functions (e.g. circle or text
// rendering might make repeated lines or rects) before ending the
// transaction. It's more efficient than starting a transaction every time.

/*!
    @brief  Draw a single pixel to the display at requested coordinates.
            Not self-contained; should follow a startWrite() call.
    @param  x      Horizontal position (0 = left).
    @param  y      Vertical position   (0 = top).
    @param  color  16-bit pixel color in '565' RGB format.
*/
void Adafruit_SPITFT::writePixel(int16_t x, int16_t y, uint16_t color) {
  if ((x >= 0) && (x < _width) && (y >= 0) && (y < _height)) {
    setAddrWindow(x, y, 1, 1);
    SPI_WRITE16(color);
  }
}

/*!
    @brief  Swap bytes in an array of pixels; converts little-to-big or
            big-to-little endian. Used by writePixels() below in some
            situations, but may also be helpful for user code occasionally.
    @param  src   Source address of 16-bit pixels buffer.
    @param  len   Number of pixels to byte-swap.
    @param  dest  Optional destination address if different than src --
                  otherwise, if NULL (default) or same address is passed,
                  pixel buffer is overwritten in-place.
*/
void Adafruit_SPITFT::swapBytes(uint16_t *src, uint32_t len, uint16_t *dest) {
  if (!dest)
    dest = src; // NULL -> overwrite src buffer
  for (uint32_t i = 0; i < len; i++) {
    dest[i] = __builtin_bswap16(src[i]);
  }
}

/*!
    @brief  Issue a series of pixels from memory to the display. Not self-
            contained; should follow startWrite() and setAddrWindow() calls.
    @param  colors     Pointer to array of 16-bit pixel values in '565' RGB
                       format.
    @param  len        Number of elements in 'colors' array.
    @param  block      If true (default case if unspecified), function blocks
                       until DMA transfer is complete. This is simply IGNORED
                       if DMA is not enabled. If false, the function returns
                       immediately after the last DMA transfer is started,
                       and one should use the dmaWait() function before
                       doing ANY other display-related activities (or even
                       any SPI-related activities, if using an SPI display
                       that shares the bus with other devices).
    @param  bigEndian  If true, bitmap in memory is in big-endian order (most
                       significant byte first). By default this is false, as
                       most microcontrollers seem to be little-endian and
                       16-bit pixel values must be byte-swapped before
                       issuing to the display (which tend toward big-endian
                       when using SPI or 8-bit parallel). If an application
                       can optimize around this -- for example, a bitmap in a
                       uint16_t array having the byte values already ordered
                       big-endian, this can save time here, ESPECIALLY if
                       using this function's non-blocking DMA mode.
*/
void Adafruit_SPITFT::writePixels(uint16_t *colors, uint32_t len, bool block,
                                  bool bigEndian) {

  if (!len)
    return; // Avoid 0-byte transfers

  // avoid paramater-not-used complaints
  (void)block;
  (void)bigEndian;

  // All other cases (bitbang SPI or non-DMA hard SPI or parallel),
  // use a loop with the normal 16-bit data write function:

  if (!bigEndian) {
    while (len--) {
      SPI_WRITE16(*colors++);
    }
  } else {
    // Well this is awkward. SPI_WRITE16() was designed for little-endian
    // hosts and big-endian displays as that's nearly always the typical
    // case. If the bigEndian flag was set, data is already in display's
    // order...so each pixel needs byte-swapping before being issued.
    // Rather than having a separate big-endian SPI_WRITE16 (adding more
    // bloat), it's preferred if calling function is smart and only uses
    // bigEndian where DMA is supported. But we gotta handle this...
    while (len--) {
      SPI_WRITE16(__builtin_bswap16(*colors++));
    }
  }
}

/*!
    @brief  Wait for the last DMA transfer in a prior non-blocking
            writePixels() call to complete. This does nothing if DMA
            is not enabled, and is not needed if blocking writePixels()
            was used (as is the default case).
*/
void Adafruit_SPITFT::dmaWait(void) {

}

/*!
    @brief  Check if DMA transfer is active. Always returts false if DMA
            is not enabled.
    @return true if DMA is enabled and transmitting data, false otherwise.
*/
bool Adafruit_SPITFT::dmaBusy(void) const {
#if defined(USE_SPI_DMA) && (defined(__SAMD51__) || defined(ARDUINO_SAMD_ZERO))
  return dma_busy;
#else
  return false;
#endif
}

/*!
    @brief  Issue a series of pixels, all the same color. Not self-
            contained; should follow startWrite() and setAddrWindow() calls.
    @param  color  16-bit pixel color in '565' RGB format.
    @param  len    Number of pixels to draw.
*/
void Adafruit_SPITFT::writeColor(uint16_t color, uint32_t len) {

  if (!len)
    return; // Avoid 0-byte transfers

  uint8_t data[2] = {(uint8_t)(color >> 8), (uint8_t)(color & 0xFF)};

  // All other cases (non-DMA hard SPI, bitbang SPI, parallel)...

  #define COLOR_FRAME_BUF_SIZE 32 // Write 32 pixels at a time

  static uint16_t tempColors[COLOR_FRAME_BUF_SIZE];

  // Set the main buffer length based on whether more pixels than the specified maximum will be drawn
  uint16_t bufLength = (len < COLOR_FRAME_BUF_SIZE) ? len : COLOR_FRAME_BUF_SIZE, xferLength;
  
  for(uint8_t i = 0; i < bufLength; i++) {
    tempColors[i] = data[1] << 8 | data[0]; // Swap byte order so that data will be sent in big-endian order to display
  }

  while (len > 0) {
    xferLength = (len < COLOR_FRAME_BUF_SIZE) ? len : COLOR_FRAME_BUF_SIZE;
    spiWriteData((uint8_t *)tempColors, xferLength * 2);
    len -= xferLength;
  }
}

/*!
    @brief  Draw a filled rectangle to the display. Not self-contained;
            should follow startWrite(). Typically used by higher-level
            graphics primitives; user code shouldn't need to call this and
            is likely to use the self-contained fillRect() instead.
            writeFillRect() performs its own edge clipping and rejection;
            see writeFillRectPreclipped() for a more 'raw' implementation.
    @param  x      Horizontal position of first corner.
    @param  y      Vertical position of first corner.
    @param  w      Rectangle width in pixels (positive = right of first
                   corner, negative = left of first corner).
    @param  h      Rectangle height in pixels (positive = below first
                   corner, negative = above first corner).
    @param  color  16-bit fill color in '565' RGB format.
    @note   Written in this deep-nested way because C by definition will
            optimize for the 'if' case, not the 'else' -- avoids branches
            and rejects clipped rectangles at the least-work possibility.
*/
void Adafruit_SPITFT::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                                    uint16_t color) {
  if (w && h) {   // Nonzero width and height?
    if (w < 0) {  // If negative width...
      x += w + 1; //   Move X to left edge
      w = -w;     //   Use positive width
    }
    if (x < _width) { // Not off right
      if (h < 0) {    // If negative height...
        y += h + 1;   //   Move Y to top edge
        h = -h;       //   Use positive height
      }
      if (y < _height) { // Not off bottom
        int16_t x2 = x + w - 1;
        if (x2 >= 0) { // Not off left
          int16_t y2 = y + h - 1;
          if (y2 >= 0) { // Not off top
            // Rectangle partly or fully overlaps screen
            if (x < 0) {
              x = 0;
              w = x2 + 1;
            } // Clip left
            if (y < 0) {
              y = 0;
              h = y2 + 1;
            } // Clip top
            if (x2 >= _width) {
              w = _width - x;
            } // Clip right
            if (y2 >= _height) {
              h = _height - y;
            } // Clip bottom
            writeFillRectPreclipped(x, y, w, h, color);
          }
        }
      }
    }
  }
}

/*!
    @brief  Draw a horizontal line on the display. Performs edge clipping
            and rejection. Not self-contained; should follow startWrite().
            Typically used by higher-level graphics primitives; user code
            shouldn't need to call this and is likely to use the self-
            contained drawFastHLine() instead.
    @param  x      Horizontal position of first point.
    @param  y      Vertical position of first point.
    @param  w      Line width in pixels (positive = right of first point,
                   negative = point of first corner).
    @param  color  16-bit line color in '565' RGB format.
*/
void inline Adafruit_SPITFT::writeFastHLine(int16_t x, int16_t y, int16_t w,
                                            uint16_t color) {
  if ((y >= 0) && (y < _height) && w) { // Y on screen, nonzero width
    if (w < 0) {                        // If negative width...
      x += w + 1;                       //   Move X to left edge
      w = -w;                           //   Use positive width
    }
    if (x < _width) { // Not off right
      int16_t x2 = x + w - 1;
      if (x2 >= 0) { // Not off left
        // Line partly or fully overlaps screen
        if (x < 0) {
          x = 0;
          w = x2 + 1;
        } // Clip left
        if (x2 >= _width) {
          w = _width - x;
        } // Clip right
        writeFillRectPreclipped(x, y, w, 1, color);
      }
    }
  }
}

/*!
    @brief  Draw a vertical line on the display. Performs edge clipping and
            rejection. Not self-contained; should follow startWrite().
            Typically used by higher-level graphics primitives; user code
            shouldn't need to call this and is likely to use the self-
            contained drawFastVLine() instead.
    @param  x      Horizontal position of first point.
    @param  y      Vertical position of first point.
    @param  h      Line height in pixels (positive = below first point,
                   negative = above first point).
    @param  color  16-bit line color in '565' RGB format.
*/
void inline Adafruit_SPITFT::writeFastVLine(int16_t x, int16_t y, int16_t h,
                                            uint16_t color) {
  if ((x >= 0) && (x < _width) && h) { // X on screen, nonzero height
    if (h < 0) {                       // If negative height...
      y += h + 1;                      //   Move Y to top edge
      h = -h;                          //   Use positive height
    }
    if (y < _height) { // Not off bottom
      int16_t y2 = y + h - 1;
      if (y2 >= 0) { // Not off top
        // Line partly or fully overlaps screen
        if (y < 0) {
          y = 0;
          h = y2 + 1;
        } // Clip top
        if (y2 >= _height) {
          h = _height - y;
        } // Clip bottom
        writeFillRectPreclipped(x, y, 1, h, color);
      }
    }
  }
}

/*!
    @brief  A lower-level version of writeFillRect(). This version requires
            all inputs are in-bounds, that width and height are positive,
            and no part extends offscreen. NO EDGE CLIPPING OR REJECTION IS
            PERFORMED. If higher-level graphics primitives are written to
            handle their own clipping earlier in the drawing process, this
            can avoid unnecessary function calls and repeated clipping
            operations in the lower-level functions.
    @param  x      Horizontal position of first corner. MUST BE WITHIN
                   SCREEN BOUNDS.
    @param  y      Vertical position of first corner. MUST BE WITHIN SCREEN
                   BOUNDS.
    @param  w      Rectangle width in pixels. MUST BE POSITIVE AND NOT
                   EXTEND OFF SCREEN.
    @param  h      Rectangle height in pixels. MUST BE POSITIVE AND NOT
                   EXTEND OFF SCREEN.
    @param  color  16-bit fill color in '565' RGB format.
    @note   This is a new function, no graphics primitives besides rects
            and horizontal/vertical lines are written to best use this yet.
*/
inline void Adafruit_SPITFT::writeFillRectPreclipped(int16_t x, int16_t y,
                                                     int16_t w, int16_t h,
                                                     uint16_t color) {
  setAddrWindow(x, y, w, h);
  writeColor(color, (uint32_t)w * h);
}

// -------------------------------------------------------------------------
// Ever-so-slightly higher-level graphics operations. Similar to the 'write'
// functions above, but these contain their own chip-select and SPI
// transactions as needed (via startWrite(), endWrite()). They're typically
// used solo -- as graphics primitives in themselves, not invoked by higher-
// level primitives (which should use the functions above for better
// performance).

/*!
    @brief  Draw a single pixel to the display at requested coordinates.
            Self-contained and provides its own transaction as needed
            (see writePixel(x,y,color) for a lower-level variant).
            Edge clipping is performed here.
    @param  x      Horizontal position (0 = left).
    @param  y      Vertical position   (0 = top).
    @param  color  16-bit pixel color in '565' RGB format.
*/
void Adafruit_SPITFT::drawPixel(int16_t x, int16_t y, uint16_t color) {
  // Clip first...
  if ((x >= 0) && (x < _width) && (y >= 0) && (y < _height)) {
    // THEN set up transaction (if needed) and draw...
    startWrite();
    setAddrWindow(x, y, 1, 1);
    SPI_WRITE16(color);
    endWrite();
  }
}

/*!
    @brief  Draw a filled rectangle to the display. Self-contained and
            provides its own transaction as needed (see writeFillRect() or
            writeFillRectPreclipped() for lower-level variants). Edge
            clipping and rejection is performed here.
    @param  x      Horizontal position of first corner.
    @param  y      Vertical position of first corner.
    @param  w      Rectangle width in pixels (positive = right of first
                   corner, negative = left of first corner).
    @param  h      Rectangle height in pixels (positive = below first
                   corner, negative = above first corner).
    @param  color  16-bit fill color in '565' RGB format.
    @note   This repeats the writeFillRect() function almost in its entirety,
            with the addition of a transaction start/end. It's done this way
            (rather than starting the transaction and calling writeFillRect()
            to handle clipping and so forth) so that the transaction isn't
            performed at all if the rectangle is rejected. It's really not
            that much code.
*/
void Adafruit_SPITFT::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                               uint16_t color) {
  if (w && h) {   // Nonzero width and height?
    if (w < 0) {  // If negative width...
      x += w + 1; //   Move X to left edge
      w = -w;     //   Use positive width
    }
    if (x < _width) { // Not off right
      if (h < 0) {    // If negative height...
        y += h + 1;   //   Move Y to top edge
        h = -h;       //   Use positive height
      }
      if (y < _height) { // Not off bottom
        int16_t x2 = x + w - 1;
        if (x2 >= 0) { // Not off left
          int16_t y2 = y + h - 1;
          if (y2 >= 0) { // Not off top
            // Rectangle partly or fully overlaps screen
            if (x < 0) {
              x = 0;
              w = x2 + 1;
            } // Clip left
            if (y < 0) {
              y = 0;
              h = y2 + 1;
            } // Clip top
            if (x2 >= _width) {
              w = _width - x;
            } // Clip right
            if (y2 >= _height) {
              h = _height - y;
            } // Clip bottom
            startWrite();
            writeFillRectPreclipped(x, y, w, h, color);
            endWrite();
          }
        }
      }
    }
  }
}

/*!
    @brief  Draw a horizontal line on the display. Self-contained and
            provides its own transaction as needed (see writeFastHLine() for
            a lower-level variant). Edge clipping and rejection is performed
            here.
    @param  x      Horizontal position of first point.
    @param  y      Vertical position of first point.
    @param  w      Line width in pixels (positive = right of first point,
                   negative = point of first corner).
    @param  color  16-bit line color in '565' RGB format.
    @note   This repeats the writeFastHLine() function almost in its
            entirety, with the addition of a transaction start/end. It's
            done this way (rather than starting the transaction and calling
            writeFastHLine() to handle clipping and so forth) so that the
            transaction isn't performed at all if the line is rejected.
*/
void Adafruit_SPITFT::drawFastHLine(int16_t x, int16_t y, int16_t w,
                                    uint16_t color) {
  if ((y >= 0) && (y < _height) && w) { // Y on screen, nonzero width
    if (w < 0) {                        // If negative width...
      x += w + 1;                       //   Move X to left edge
      w = -w;                           //   Use positive width
    }
    if (x < _width) { // Not off right
      int16_t x2 = x + w - 1;
      if (x2 >= 0) { // Not off left
        // Line partly or fully overlaps screen
        if (x < 0) {
          x = 0;
          w = x2 + 1;
        } // Clip left
        if (x2 >= _width) {
          w = _width - x;
        } // Clip right
        startWrite();
        writeFillRectPreclipped(x, y, w, 1, color);
        endWrite();
      }
    }
  }
}

/*!
    @brief  Draw a vertical line on the display. Self-contained and provides
            its own transaction as needed (see writeFastHLine() for a lower-
            level variant). Edge clipping and rejection is performed here.
    @param  x      Horizontal position of first point.
    @param  y      Vertical position of first point.
    @param  h      Line height in pixels (positive = below first point,
                   negative = above first point).
    @param  color  16-bit line color in '565' RGB format.
    @note   This repeats the writeFastVLine() function almost in its
            entirety, with the addition of a transaction start/end. It's
            done this way (rather than starting the transaction and calling
            writeFastVLine() to handle clipping and so forth) so that the
            transaction isn't performed at all if the line is rejected.
*/
void Adafruit_SPITFT::drawFastVLine(int16_t x, int16_t y, int16_t h,
                                    uint16_t color) {
  if ((x >= 0) && (x < _width) && h) { // X on screen, nonzero height
    if (h < 0) {                       // If negative height...
      y += h + 1;                      //   Move Y to top edge
      h = -h;                          //   Use positive height
    }
    if (y < _height) { // Not off bottom
      int16_t y2 = y + h - 1;
      if (y2 >= 0) { // Not off top
        // Line partly or fully overlaps screen
        if (y < 0) {
          y = 0;
          h = y2 + 1;
        } // Clip top
        if (y2 >= _height) {
          h = _height - y;
        } // Clip bottom
        startWrite();
        writeFillRectPreclipped(x, y, 1, h, color);
        endWrite();
      }
    }
  }
}

/*!
    @brief  Essentially writePixel() with a transaction around it. I don't
            think this is in use by any of our code anymore (believe it was
            for some older BMP-reading examples), but is kept here in case
            any user code relies on it. Consider it DEPRECATED.
    @param  color  16-bit pixel color in '565' RGB format.
*/
void Adafruit_SPITFT::pushColor(uint16_t color) {
  startWrite();
  SPI_WRITE16(color);
  endWrite();
}

/*!
    @brief  Draw a 16-bit image (565 RGB) at the specified (x,y) position.
            For 16-bit display devices; no color reduction performed.
            Adapted from https://github.com/PaulStoffregen/ILI9341_t3
            by Marc MERLIN. See examples/pictureEmbed to use this.
            5/6/2017: function name and arguments have changed for
            compatibility with current GFX library and to avoid naming
            problems in prior implementation.  Formerly drawBitmap() with
            arguments in different order. Handles its own transaction and
            edge clipping/rejection.
    @param  x        Top left corner horizontal coordinate.
    @param  y        Top left corner vertical coordinate.
    @param  pcolors  Pointer to 16-bit array of pixel values.
    @param  w        Width of bitmap in pixels.
    @param  h        Height of bitmap in pixels.
*/
void Adafruit_SPITFT::drawRGBBitmap(int16_t x, int16_t y, uint16_t *pcolors,
                                    int16_t w, int16_t h) {

  int16_t x2, y2;                 // Lower-right coord
  if ((x >= _width) ||            // Off-edge right
      (y >= _height) ||           // " top
      ((x2 = (x + w - 1)) < 0) || // " left
      ((y2 = (y + h - 1)) < 0))
    return; // " bottom

  int16_t bx1 = 0, by1 = 0, // Clipped top-left within bitmap
      saveW = w;            // Save original bitmap width value
  if (x < 0) {              // Clip left
    w += x;
    bx1 = -x;
    x = 0;
  }
  if (y < 0) { // Clip top
    h += y;
    by1 = -y;
    y = 0;
  }
  if (x2 >= _width)
    w = _width - x; // Clip right
  if (y2 >= _height)
    h = _height - y; // Clip bottom

  pcolors += by1 * saveW + bx1; // Offset bitmap ptr to clipped top-left
  startWrite();
  setAddrWindow(x, y, w, h); // Clipped area
  while (h--) {              // For each (clipped) scanline...
    writePixels(pcolors, w); // Push one (clipped) row
    pcolors += saveW;        // Advance pointer by one full (unclipped) line
  }
  endWrite();
}

// -------------------------------------------------------------------------
// Miscellaneous class member functions that don't draw anything.

/*!
    @brief  Invert the colors of the display (if supported by hardware).
            Self-contained, no transaction setup required.
    @param  i  true = inverted display, false = normal display.
*/
void Adafruit_SPITFT::invertDisplay(bool i) {
  startWrite();
  writeCommand(i ? invertOnCommand : invertOffCommand);
  endWrite();
}

/*!
    @brief   Given 8-bit red, green and blue values, return a 'packed'
             16-bit color value in '565' RGB format (5 bits red, 6 bits
             green, 5 bits blue). This is just a mathematical operation,
             no hardware is touched.
    @param   red    8-bit red brightnesss (0 = off, 255 = max).
    @param   green  8-bit green brightnesss (0 = off, 255 = max).
    @param   blue   8-bit blue brightnesss (0 = off, 255 = max).
    @return  'Packed' 16-bit color value (565 format).
*/
uint16_t Adafruit_SPITFT::color565(uint8_t red, uint8_t green, uint8_t blue) {
  return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
}

/*!
@brief   Adafruit_SPITFT Send Command handles complete sending of commands and
data
@param   commandByte       The Command Byte
@param   dataBytes         A pointer to the Data bytes to send
@param   numDataBytes      The number of bytes we should send
*/
void Adafruit_SPITFT::sendCommand(uint8_t commandByte, uint8_t *dataBytes,
                                  uint8_t numDataBytes) {
  // SPI_BEGIN_TRANSACTION();
  if (_cs_pin >= 0)
    SPI_CS_LOW();

  SPI_DC_LOW();          // Command mode
  spiWrite(commandByte); // Send the command byte

  SPI_DC_HIGH();
  spiWriteData(dataBytes, numDataBytes);
  // for (int i = 0; i < numDataBytes; i++) {
  //   spiWrite(*dataBytes); // Send the data bytes
  //   dataBytes++;
  // }

  if (_cs_pin >= 0)
    SPI_CS_HIGH();
  // SPI_END_TRANSACTION();
}

/*!
 @brief   Adafruit_SPITFT Send Command handles complete sending of commands and
 data
 @param   commandByte       The Command Byte
 @param   dataBytes         A pointer to the Data bytes to send
 @param   numDataBytes      The number of bytes we should send
 */
void Adafruit_SPITFT::sendCommand(uint8_t commandByte, const uint8_t *dataBytes,
                                  uint8_t numDataBytes) {
  // SPI_BEGIN_TRANSACTION();
  if (_cs_pin >= 0)
    SPI_CS_LOW();

  SPI_DC_LOW();          // Command mode
  spiWrite(commandByte); // Send the command byte

  SPI_DC_HIGH();
  if (numDataBytes > 0) spiWriteData(dataBytes, numDataBytes);
  // for (int i = 0; i < numDataBytes; i++) {
  //   spiWrite(pgm_read_byte(dataBytes++));
  // }

  if (_cs_pin >= 0)
    SPI_CS_HIGH();
  // SPI_END_TRANSACTION();
}

/*!
 @brief  Adafruit_SPITFT sendCommand16 handles complete sending of
         commands and data for 16-bit parallel displays. Currently somewhat
         rigged for the NT35510, which has the odd behavior of wanting
         commands 16-bit, but subsequent data as 8-bit values, despite
         the 16-bit bus (high byte is always 0). Also seems to require
         issuing and incrementing address with each transfer.
 @param  commandWord   The command word (16 bits)
 @param  dataBytes     A pointer to the data bytes to send
 @param  numDataBytes  The number of bytes we should send
 */
void Adafruit_SPITFT::sendCommand16(uint16_t commandWord,
                                    const uint8_t *dataBytes,
                                    uint8_t numDataBytes) {
  // SPI_BEGIN_TRANSACTION();
  if (_cs_pin >= 0)
    SPI_CS_LOW();

  if (numDataBytes == 0) {
    SPI_DC_LOW();             // Command mode
    SPI_WRITE16(commandWord); // Send the command word
    SPI_DC_HIGH();            // Data mode
  }
  for (int i = 0; i < numDataBytes; i++) {
    SPI_DC_LOW();             // Command mode
    SPI_WRITE16(commandWord); // Send the command word
    SPI_DC_HIGH();            // Data mode
    commandWord++;
    SPI_WRITE16((uint16_t)*(dataBytes++));
  }

  if (_cs_pin >= 0)
    SPI_CS_HIGH();
  // SPI_END_TRANSACTION();
}

// /*!
//  @brief   Read 8 bits of data from display configuration memory (not RAM).
//  This is highly undocumented/supported and should be avoided,
//  function is only included because some of the examples use it.
//  @param   commandByte
//  The command register to read data from.
//  @param   index
//  The byte index into the command to read from.
//  @return  Unsigned 8-bit data read from display register.
//  */
// /**************************************************************************/
// uint8_t Adafruit_SPITFT::readcommand8(uint8_t commandByte, uint8_t index) {
//   uint8_t result;
//   startWrite();
//   SPI_DC_LOW(); // Command mode
//   spiWrite(commandByte);
//   SPI_DC_HIGH(); // Data mode
//   do {
//     result = spiRead();
//   } while (index--); // Discard bytes up to index'th
//   endWrite();
//   return result;
// }

/*!
 @brief   Read 16 bits of data from display register.
          For 16-bit parallel displays only.
 @param   addr  Command/register to access.
 @return  Unsigned 16-bit data.
 */
// uint16_t Adafruit_SPITFT::readcommand16(uint16_t addr) {
// #if defined(USE_FAST_PINIO) // NOT SUPPORTED without USE_FAST_PINIO
//   uint16_t result = 0;
//   if ((connection == TFT_PARALLEL) && tft8.wide) {
//     startWrite();
//     SPI_DC_LOW(); // Command mode
//     SPI_WRITE16(addr);
//     SPI_DC_HIGH(); // Data mode
//     TFT_RD_LOW();  // Read line LOW
// #if defined(HAS_PORT_SET_CLR)
//     *(volatile uint16_t *)tft8.dirClr = 0xFFFF;   // Input state
//     result = *(volatile uint16_t *)tft8.readPort; // 16-bit read
//     *(volatile uint16_t *)tft8.dirSet = 0xFFFF;   // Output state
// #else                                             // !HAS_PORT_SET_CLR
//     *(volatile uint16_t *)tft8.portDir = 0x0000;  // Input state
//     result = *(volatile uint16_t *)tft8.readPort; // 16-bit read
//     *(volatile uint16_t *)tft8.portDir = 0xFFFF;  // Output state
// #endif                                            // end !HAS_PORT_SET_CLR
//     TFT_RD_HIGH();                                // Read line HIGH
//     endWrite();
//   }
//   return result;
// #else
//   (void)addr; // disable -Wunused-parameter warning
//   return 0;
// #endif // end !USE_FAST_PINIO
// }

// -------------------------------------------------------------------------
// Lowest-level hardware-interfacing functions. Many of these are inline and
// compile to different things based on #defines -- typically just a few
// instructions. Others, not so much, those are not inlined.

/*!
    @brief  Start an SPI transaction if using the hardware SPI interface to
            the display. If using an earlier version of the Arduino platform
            (before the addition of SPI transactions), this instead attempts
            to set up the SPI clock and mode. No action is taken if the
            connection is not hardware SPI-based. This does NOT include a
            chip-select operation -- see startWrite() for a function that
            encapsulated both actions.
*/
// inline void Adafruit_SPITFT::SPI_BEGIN_TRANSACTION(void) {
//   if (connection == TFT_HARD_SPI) {
// #if defined(SPI_HAS_TRANSACTION)
//     hwspi._spi->beginTransaction(hwspi.settings);
// #else // No transactions, configure SPI manually...
// #if defined(__AVR__) || defined(TEENSYDUINO) || defined(ARDUINO_ARCH_STM32F1)
//     hwspi._spi->setClockDivider(SPI_CLOCK_DIV2);
// #elif defined(__arm__)
//     hwspi._spi->setClockDivider(11);
// #elif defined(ESP8266) || defined(ESP32)
//     hwspi._spi->setFrequency(hwspi._freq);
// #elif defined(RASPI) || defined(ARDUINO_ARCH_STM32F1)
//     hwspi._spi->setClock(hwspi._freq);
// #endif
//     hwspi._spi->setBitOrder(MSBFIRST);
//     hwspi._spi->setDataMode(hwspi._mode);
// #endif // end !SPI_HAS_TRANSACTION
//   }
// }

/*!
    @brief  End an SPI transaction if using the hardware SPI interface to
            the display. No action is taken if the connection is not
            hardware SPI-based or if using an earlier version of the Arduino
            platform (before the addition of SPI transactions). This does
            NOT include a chip-deselect operation -- see endWrite() for a
            function that encapsulated both actions.
*/
// inline void Adafruit_SPITFT::SPI_END_TRANSACTION(void) {
// #if defined(SPI_HAS_TRANSACTION)
//   if (connection == TFT_HARD_SPI) {
//     hwspi._spi->endTransaction();
//   }
// #endif
// }

/*!
    @brief  Issue a single 8-bit value to the display. Chip-select,
            transaction and data/command selection must have been
            previously set -- this ONLY issues the byte. This is another of
            those functions in the library with a now-not-accurate name
            that's being maintained for compatibility with outside code.
            This function is used even if display connection is parallel.
    @param  b  8-bit value to write.
*/
void Adafruit_SPITFT::spiWrite(uint8_t b) {
  *hwspi._spiTxDone = false; // Clear flag before starting SPI transmission
  HAL_SPI_Transmit_DMA(hwspi._spi, &b, 1);

  while (!(*hwspi._spiTxDone)); // Wait for the SPI transmission to complete
  while(!__HAL_SPI_GET_FLAG(hwspi._spi, SPI_FLAG_TXC)); // Wait for the SPI peripheral to free up after transmission
}

void Adafruit_SPITFT::spiWriteData(uint8_t *data, uint32_t len) {
  spiWriteData((const uint8_t *)data, len);
}

void Adafruit_SPITFT::spiWriteData(const uint8_t *data, uint32_t len) {
  *hwspi._spiTxDone = false; // Clear flag before starting SPI transmission
  HAL_SPI_Transmit_DMA(hwspi._spi, data, len);

  while (!(*hwspi._spiTxDone)); // Wait for the SPI transmission to complete
  while(!__HAL_SPI_GET_FLAG(hwspi._spi, SPI_FLAG_TXC)); // Wait for the SPI peripheral to free up after transmission
}

/*!
    @brief  Write a single command byte to the display. Chip-select and
            transaction must have been previously set -- this ONLY sets
            the device to COMMAND mode, issues the byte and then restores
            DATA mode. There is no corresponding explicit writeData()
            function -- just use spiWrite().
    @param  cmd  8-bit command to write.
*/
void Adafruit_SPITFT::writeCommand(uint8_t cmd) {
  SPI_DC_LOW();
  spiWrite(cmd);
  SPI_DC_HIGH();
}

// /*!
//     @brief   Read a single 8-bit value from the display. Chip-select and
//              transaction must have been previously set -- this ONLY reads
//              the byte. This is another of those functions in the library
//              with a now-not-accurate name that's being maintained for
//              compatibility with outside code. This function is used even if
//              display connection is parallel.
//     @return  Unsigned 8-bit value read (always zero if USE_FAST_PINIO is
//              not supported by the MCU architecture).
// */
// uint8_t Adafruit_SPITFT::spiRead(void) {
//   uint8_t b = 0;
//   uint16_t w = 0;
//   return hwspi._spi->transfer((uint8_t)0);
// }

/*!
    @brief  Issue a single 16-bit value to the display. Chip-select,
            transaction and data/command selection must have been
            previously set -- this ONLY issues the word.
            Thus operates ONLY on 'wide' (16-bit) parallel displays!
    @param  w  16-bit value to write.
*/
// void Adafruit_SPITFT::write16(uint16_t w) {
//   if (connection == TFT_PARALLEL) {
// #if defined(USE_FAST_PINIO)
//     if (tft8.wide)
//       *(volatile uint16_t *)tft8.writePort = w;
// #else
//     (void)w; // disable -Wunused-parameter warning
// #endif
//     TFT_WR_STROBE();
//   }
// }

/*!
    @brief  Write a single command word to the display. Chip-select and
            transaction must have been previously set -- this ONLY sets
            the device to COMMAND mode, issues the byte and then restores
            DATA mode. This operates ONLY on 'wide' (16-bit) parallel
            displays!
    @param  cmd  16-bit command to write.
*/
// void Adafruit_SPITFT::writeCommand16(uint16_t cmd) {
//   SPI_DC_LOW();
//   write16(cmd);
//   SPI_DC_HIGH();
// }

/*!
    @brief   Read a single 16-bit value from the display. Chip-select and
             transaction must have been previously set -- this ONLY reads
             the byte. This operates ONLY on 'wide' (16-bit) parallel
             displays!
    @return  Unsigned 16-bit value read (always zero if USE_FAST_PINIO is
             not supported by the MCU architecture).
*/
// uint16_t Adafruit_SPITFT::read16(void) {
//   uint16_t w = 0;
//   if (connection == TFT_PARALLEL) {
//     if (tft8._rd >= 0) {
// #if defined(USE_FAST_PINIO)
//       TFT_RD_LOW();    // Read line LOW
//       if (tft8.wide) { // 16-bit TFT connection
// #if defined(HAS_PORT_SET_CLR)
//         *(volatile uint16_t *)tft8.dirClr = 0xFFFF; // Input state
//         w = *(volatile uint16_t *)tft8.readPort;    // 16-bit read
//         *(volatile uint16_t *)tft8.dirSet = 0xFFFF; // Output state
// #else                                               // !HAS_PORT_SET_CLR
//         *(volatile uint16_t *)tft8.portDir = 0x0000; // Input state
//         w = *(volatile uint16_t *)tft8.readPort;     // 16-bit read
//         *(volatile uint16_t *)tft8.portDir = 0xFFFF; // Output state
// #endif                                              // end !HAS_PORT_SET_CLR
//       }
//       TFT_RD_HIGH(); // Read line HIGH
// #else                // !USE_FAST_PINIO
//       w = 0; // Parallel TFT is NOT SUPPORTED without USE_FAST_PINIO
// #endif               // end !USE_FAST_PINIO
//     }
//   }
//   return w;
// }

/*!
    @brief  Set the software (bitbang) SPI MOSI line HIGH.
*/
// inline void Adafruit_SPITFT::SPI_MOSI_HIGH(void) {
// #if defined(USE_FAST_PINIO)
// #if defined(HAS_PORT_SET_CLR)
// #if defined(KINETISK)
//   *swspi.mosiPortSet = 1;
// #else // !KINETISK
//   *swspi.mosiPortSet = swspi.mosiPinMask;
// #endif
// #else  // !HAS_PORT_SET_CLR
//   *swspi.mosiPort |= swspi.mosiPinMaskSet;
// #endif // end !HAS_PORT_SET_CLR
// #else  // !USE_FAST_PINIO
//   digitalWrite(swspi._mosi, HIGH);
// #endif // end !USE_FAST_PINIO
// }

/*!
    @brief  Set the software (bitbang) SPI MOSI line LOW.
*/
// inline void Adafruit_SPITFT::SPI_MOSI_LOW(void) {
// #if defined(USE_FAST_PINIO)
// #if defined(HAS_PORT_SET_CLR)
// #if defined(KINETISK)
//   *swspi.mosiPortClr = 1;
// #else // !KINETISK
//   *swspi.mosiPortClr = swspi.mosiPinMask;
// #endif
// #else  // !HAS_PORT_SET_CLR
//   *swspi.mosiPort &= swspi.mosiPinMaskClr;
// #endif // end !HAS_PORT_SET_CLR
// #else  // !USE_FAST_PINIO
//   digitalWrite(swspi._mosi, LOW);
// #endif // end !USE_FAST_PINIO
// }

/*!
    @brief  Set the software (bitbang) SPI SCK line HIGH.
*/
// inline void Adafruit_SPITFT::SPI_SCK_HIGH(void) {
// #if defined(USE_FAST_PINIO)
// #if defined(HAS_PORT_SET_CLR)
// #if defined(KINETISK)
//   *swspi.sckPortSet = 1;
// #else // !KINETISK
//   *swspi.sckPortSet = swspi.sckPinMask;
// #endif
// #else  // !HAS_PORT_SET_CLR
//   *swspi.sckPort |= swspi.sckPinMaskSet;
// #endif // end !HAS_PORT_SET_CLR
// #else  // !USE_FAST_PINIO
//   digitalWrite(swspi._sck, HIGH);
// #endif // end !USE_FAST_PINIO
// }

/*!
    @brief  Set the software (bitbang) SPI SCK line LOW.
*/
// inline void Adafruit_SPITFT::SPI_SCK_LOW(void) {
// #if defined(USE_FAST_PINIO)
// #if defined(HAS_PORT_SET_CLR)
// #if defined(KINETISK)
//   *swspi.sckPortClr = 1;
// #else // !KINETISK
//   *swspi.sckPortClr = swspi.sckPinMask;
// #endif
// #else  // !HAS_PORT_SET_CLR
//   *swspi.sckPort &= swspi.sckPinMaskClr;
// #endif // end !HAS_PORT_SET_CLR
// #else  // !USE_FAST_PINIO
//   digitalWrite(swspi._sck, LOW);
// #endif // end !USE_FAST_PINIO
// }

/*!
    @brief   Read the state of the software (bitbang) SPI MISO line.
    @return  true if HIGH, false if LOW.
*/
// inline bool Adafruit_SPITFT::SPI_MISO_READ(void) {
// #if defined(USE_FAST_PINIO)
// #if defined(KINETISK)
//   return *swspi.misoPort;
// #else  // !KINETISK
//   return *swspi.misoPort & swspi.misoPinMask;
// #endif // end !KINETISK
// #else  // !USE_FAST_PINIO
//   return digitalRead(swspi._miso);
// #endif // end !USE_FAST_PINIO
// }

/*!
    @brief  Issue a single 16-bit value to the display. Chip-select,
            transaction and data/command selection must have been
            previously set -- this ONLY issues the word. Despite the name,
            this function is used even if display connection is parallel;
            name was maintaned for backward compatibility. Naming is also
            not consistent with the 8-bit version, spiWrite(). Sorry about
            that. Again, staying compatible with outside code.
    @param  w  16-bit value to write.
*/
void Adafruit_SPITFT::SPI_WRITE16(uint16_t w) {
  union {
    uint16_t val;
    uint8_t bytes[2];
  } data;

  data.val = w;

  flip_endianness(data.bytes, 2);

  spiWriteData(data.bytes, 2);
}

/*!
    @brief  Issue a single 32-bit value to the display. Chip-select,
            transaction and data/command selection must have been
            previously set -- this ONLY issues the longword. Despite the
            name, this function is used even if display connection is
            parallel; name was maintaned for backward compatibility. Naming
            is also not consistent with the 8-bit version, spiWrite().
            Sorry about that. Again, staying compatible with outside code.
    @param  l  32-bit value to write.
*/
void Adafruit_SPITFT::SPI_WRITE32(uint32_t l) {
  union {
    uint32_t val;
    uint8_t bytes[4];
  } data;

  data.val = l;

  flip_endianness(data.bytes, 4);

  spiWriteData(data.bytes, 4);
}
