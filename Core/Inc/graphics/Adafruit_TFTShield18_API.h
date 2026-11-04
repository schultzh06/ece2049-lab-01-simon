/**
 * @file Adafruit_TFTShield18_API.cpp
 * @author Kyle Schmottlach (kjschmottlach@wpi.edu)
 * @brief Wrapper class for the Adafruit TFT Shield 1.8" that provides a C API for the C++ class defined in Adafruit_TFTShield18.h. This is used to allow 
 *        the TFT shield to be used in C code. This class is intended to be used as a singleton, containing all functionality for initializing and
 *        controlling the TFT seesaw chip
 * @version 1.0
 * @date 2026-03-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef ADAFRUIT_TFTSHIELD18_API_H
#define ADAFRUIT_TFTSHIELD18_API_H

#include "stdint.h"
#include "Adafruit_TFTShield18_Macros.h"
#include "stm32h5xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Internal handle used to store the instance of an Adafruit_TFTShield18 class for use in the C API functions. This struct is not 
 *        intended to be used outside of this API and should be treated as an opaque handle. Provides a C compatible structure to store 
 *        the C++ class instance for the TFT shield.
 * 
 */
typedef struct TFTShield18_Handle TFTShield18_Handle;

/**
 * @brief Function to create an instance of the seesaw class for the TFT shield. Must be called before any other functions in this API
 * 
 * @param i2c_handle Pointer to the STM32 HAL I2C handle to use for communication with the TFT seesaw device
 */
void TFTShield18_create(I2C_HandleTypeDef *i2c_handle);

/**
 * @brief Function to initialize the seesaw device on the TFT shield. Must be called before using any other functions in this API. 
 *        Uses the default I2C address of 0x3E for the TFT seesaw device
 * 
 * @return true if the initialization was successful
 * @return false if the initialization failed
 */
bool TFTShield18_begin();

/**
 * @brief Sets the backlight to a value between 0(0x0000) (off) and 65535(0xFFFF) (full brightness)
 * 
 * @param value Value to set the backlight to (between 0 and 65535)
 */
void TFTShield18_setBacklight(uint16_t value);

/**
 * @brief Sets the frequency of the backlight PWM
 * 
 * @param freq Frequency to set the backlight PWM to
 */
void TFTShield18_setBacklightFreq(uint16_t freq);

/**
 * @brief Perform a SW reset of the display and seesaw devices. Required to be called after initialization to reset the display 
 *        from an unknown state. Should be called whenever a reset of the display is needed by setting rst to true and then back to false after a delay
 * 
 * @param rst Boolean value to trigger the reset. Set to true to initiate the reset and false to complete it.
 */
void TFTShield18_tftReset(bool rst);

/**
 * @brief Function to read the state of the on board shield buttons from the seesaw chip over I2C. Returned bitmask
 *        has bits set corresponding to the TFTSHIELD_BUTTON macros defined in Adafruit_TFTShield18_Macros.h
 * 
 * @return uint32_t Bitmask representing the state of the buttons
 */
uint32_t TFTShield18_readButtons();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ADAFRUIT_TFTSHIELD18_API_H