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

#include "Adafruit_TFTShield18_API.h"
#include "Adafruit_TFTShield18.h"

struct TFTShield18_Handle {
    Adafruit_TFTShield18 instance;
};

TFTShield18_Handle *seesaw_handle;

void TFTShield18_create(I2C_HandleTypeDef *i2c_handle) {
    static TFTShield18_Handle seesaw = TFTShield18_Handle{Adafruit_TFTShield18(i2c_handle)};
    seesaw_handle = &seesaw;
}

bool TFTShield18_begin() {
    return seesaw_handle->instance.begin(TFTSHIELD_ADDR);
}

void TFTShield18_setBacklight(uint16_t value) {
    seesaw_handle->instance.setBacklight(value);
}

void TFTShield18_setBacklightFreq(uint16_t freq) {
    seesaw_handle->instance.setBacklightFreq(freq);
}
void TFTShield18_tftReset(bool rst) {
    seesaw_handle->instance.tftReset(rst);
}
uint32_t TFTShield18_readButtons() {
    return seesaw_handle->instance.readButtons();
}
