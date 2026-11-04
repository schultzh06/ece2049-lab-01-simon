/*
* Originally written by Adafruit Industries for general-purpose Arduino development.
* Adapted for STM32 for ECE 2049 by Kyle Schmottlach and Shannon Miranda '26
*/

#ifndef _TFT_SHIELD_18_H
#define _TFT_SHIELD_18_H

#include "Adafruit_seesaw.h"
#include "Adafruit_TFTShield18_Macros.h"

/**************************************************************************/
/*!
    @brief  Class that stores state and functions for interacting with 1.8" tft
   shield variant of seesaw helper IC
*/
/**************************************************************************/
class Adafruit_TFTShield18 : public Adafruit_seesaw {
public:
  Adafruit_TFTShield18();
  Adafruit_TFTShield18(I2C_HandleTypeDef *i2c_handle);
  ~Adafruit_TFTShield18(){};

  bool begin(uint8_t addr = TFTSHIELD_ADDR);

  void setBacklight(uint16_t value);
  void setBacklightFreq(uint16_t freq);
  void tftReset(bool rst = true);
  uint32_t readButtons();
};

#endif