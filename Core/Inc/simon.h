/**
 * @file simon.h
 * @author Hayden Schultz, Frank Santen
 * @brief Public interface for the simon says game.
 * @date 2026-11-12
 */

#ifndef SIMON_H
#define SIMON_H

/**
 * @brief Reset the game and show the start screen.
 * 
 * Call once after the display, buzzer, and simon_io_init() are set up.
 */
void simon_init(void);

/**
 * @brief Run one step of the game. Call every pass of the main loop.
 *
 * Non-blocking. Select returns to the start screen from any state.
 */
void simon_update(void);

#endif