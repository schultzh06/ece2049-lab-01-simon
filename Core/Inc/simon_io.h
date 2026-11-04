/**
 * @file simon_io.h
 * @author Hayden Schultz
 * @brief Hardware layer for the Simon Says game logic.
 * @date 2026-11-04
 * 
 * Exposes functions to initialize the GPIO pins through register writes,
 * LED control, and button reading.
 * 
 */

#ifndef SIMON_IO_H
#define SIMON_IO_H

#include <stdint.h>
#include <stdbool.h>

#define NUM_COLORS 4

/**
 * @brief Configure all LED and button GPIO pins.
 * 
 * Enables the GPIOB/C clocks, sets LEDs as push-pull outputs
 * and buttons as inputs with pull-ups. Call once on start.
 */
void simon_io_init(void);

/**
 * @brief Turn one LED on or off.
 * 
 * @param i LED index, 0 to NUM_COLORS - 1
 * @param on State to set the LED to
 */
void led_set(uint8_t i, bool on);

/**
 * @brief Turn all LEDs off.
 */
void leds_all_off(void);

/**
 * @brief Check for a new button press.
 * 
 * @return int8_t Index 0 to NUM_COLORS - 1 of the button just pressed. -1 if no press.
 */
int8_t button_get_press(void);

#endif