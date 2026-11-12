/**
 * @file simon.c
 * @author Hayden Schultz, Frank Santen
 * @brief Game logic and TFT display for Simon Says.
 * @date 2026-11-12
 * 
 * Runs simon as a non blocking state machine. simon_update() is called every pass of the main loop.
 * reads one button press, and dispatches to the handler for the current state. Timing uses HAL_GetTick()
 * so the loop does not block.
 * 
 * States
 *   START: Menu, idle LED animation, SELECT starts a 3-2-1 countdown
 *   SHOW_SEQUENCE: Plays sequence on LEDs, buzzer, and on screen display
 *   WAIT_INPUT: Records and checks each button press
 *   GAME_OVER: Shows high scores, waits for SELECT (or 10s)
 *   CORRECT: Flashes all lights to show the player was corect
 */
 
#include "simon.h"
#include "Adafruit_ST77xx_Macros.h"
#include "main.h"
#include "buzzer.h"
#include "simon_io.h"
#include "stm32h5xx_hal.h"
#include "stm32h5xx_hal_def.h"
#include "stm32h5xx_hal_rng.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "Adafruit_ST7735_API.h"
#include "Adafruit_ST7735_Macros.h"
#include <string.h>

/** Game timing and sequence limit */
#define MAX_SEQUENCE 100
#define FLASH_ON_MS 500
#define FLASH_OFF_MS 200
#define PRESS_FLASH_MS 300

/** History: HIST_MAX circles centered at the bottom of the screen */
#define HIST_R 7
#define HIST_SP 18
#define HIST_Y 119
#define HIST_MAX 5

/** Status bar along top of screen */
#define BAR_H (Display_height() / 5)
#define BAR_PAD 4

extern Buzzer buzzer1;
extern RNG_HandleTypeDef hrng;

/** Game states, see file header */
typedef enum {
    STATE_START,
    STATE_SHOW_SEQUENCE,
    STATE_WAIT_INPUT,
    STATE_GAME_OVER,
    STATE_CORRECT
} SimonState;

static SimonState state = STATE_START; // Current state

/*
 * Sequence and history
 * sequence[] holds the colors the game generated, history[] holds
 * the colors the player actually pressed this level. Both use the same index,
 * so press in is correct when history[n] == sequence[n].
*/
static uint8_t sequence[MAX_SEQUENCE];
static uint8_t seq_length = 0;
static uint8_t seq_index = 0;
static uint8_t history[MAX_SEQUENCE];
static uint8_t history_len = 0;

static bool led_lit = false; // Is a sequence color currently lit
static uint32_t last_tick; // timestamp for active state's timer
static int8_t lit_color = -1; // Color lit by press or animation, -1 if none

static uint8_t high_scores[3] = {0, 0, 0};

/** Buzzer note and screen color for each button, 0-3 */
static const Note color_notes[NUM_COLORS] = {NOTE_C, NOTE_E, NOTE_G, NOTE_A};
static const uint16_t circle_colors[NUM_COLORS] = {ST77XX_GREEN, ST77XX_RED, ST77XX_BLUE, ST77XX_YELLOW};

/**
 * @brief Light one color on the LED, screen, and buzzer.
 * 
 * @param i Color index, 0 to NUM_COLORS - 1
*/
static void color_on(uint8_t i) {
    led_set(i, true);
    Display_fillRoundRect(42 + (i > 0 && i < 3 ? 40 : 0), (i > 1 ? 68 : 28), 36, 36, 5, circle_colors[i]);
    buzzerSetNote(&buzzer1, color_notes[i], 1);
    buzzerEnable(&buzzer1);
}

/**
 * @brief Turn off one color on the LED, screen, and buzzer.
 * 
 * @param i Color index, 0 to NUM_COLORS - 1
*/
static void color_off(uint8_t i) {
    led_set(i, false);
    Display_fillRoundRect(42 + (i > 0 && i < 3 ? 40 : 0), (i > 1 ? 68 : 28), 36, 36, 5, ST77XX_WHITE);
    buzzerDisable(&buzzer1);
}

/**
 * @brief Append one random color to the sequence using RNG
 * 
 * If the RNG fails the sequence is unchanged
*/
static void add_random_color(void) {
    uint32_t random;
    if (HAL_RNG_GenerateRandomNumber(&hrng, &random) == HAL_OK) {
        sequence[seq_length] = random % NUM_COLORS;
        seq_length++;
    }
}

/**
 * @brief Draw the last HIST_MAX presses as colored circles.
 * 
 * Slots are fixed and centered, so the newest press is rightmost
*/
static void draw_history(void) {
    uint8_t start = (history_len > 5) ? history_len - 5 : 0;
    for (uint8_t i = start; i < history_len; i++) {
        int16_t x0 = (Display_width() - 4 * HIST_SP) / 2;
        int16_t x = x0 + (i - start) * HIST_SP;
        Display_fillCircle(x, HIST_Y, HIST_R, circle_colors[history[i]]);
    }
}

/**
 * @brief Blank all HIST_MAX history slots
*/
static void clear_history(void) {
    for (uint8_t i = 0; i < HIST_MAX; i++) {
        int16_t x0 = (Display_width() - 4 * HIST_SP) / 2;
        int16_t x = x0 + i * HIST_SP;
        Display_fillCircle(x, HIST_Y, HIST_R, ST77XX_BLACK);
    }
}

/**
 * @brief Redraw the top status bar.
 * 
 * Shows the current level left aligned and msg right aligned
 * 
 * @param msg Single-line message
 * @param text_color Text color
 * @param bar_color Bar background color
*/
static void draw_status(const char *msg, uint16_t text_color, uint16_t bar_color) {
    Display_fillRect(0, 0, Display_width(), BAR_H, bar_color);
    Display_setTextColor(text_color);

    char lvl[8];
    sprintf(lvl, "LVL %d", seq_length);
    Display_setTextSize(1, 1);
    Display_setCursor(BAR_PAD, (BAR_H - 8) / 2);
    Display_print(lvl);

    Display_setTextSize(1, 1);
    int16_t w = (int16_t)strlen(msg) * 6;
    Display_setCursor(Display_width() - w - BAR_PAD, (BAR_H - 8) / 2);
    Display_print(msg);
}

/**
 * @brief Clear the screen and draw the base layout
 * 
 * Draws the status bar background, four light pads, and msg centered above bottom edge
 * 
 * @param msg Single-line message, empty string for none
*/
static void draw_screen(const char *msg) {
    Display_fillScreen(ST77XX_BLACK);
    Display_fillRect(0, 0, Display_width(), Display_height() / 5, ST77XX_RED);
    
    Display_fillRoundRect(42, 28, 36, 36, 5, ST77XX_WHITE);
    Display_fillRoundRect(82, 28, 36, 36, 5, ST77XX_WHITE);
    Display_fillRoundRect(42, 68, 36, 36, 5, ST77XX_WHITE);
    Display_fillRoundRect(82, 68, 36, 36, 5, ST77XX_WHITE);

    Display_setTextSize(1, 1);
    int16_t w = (int16_t)strlen(msg) * 6;
    Display_setTextColor(ST77XX_WHITE);
    Display_setCursor((Display_width() - w) / 2, HIST_Y - 4);
    Display_print(msg);
}

/**
 * @brief Switch to a new state and draw that state's screen.
 * 
 * @param next State to enter
*/
static void change_state(SimonState next) {
    state = next;

    switch(next) {
        case STATE_START: 
            draw_screen("Press SELECT!"); 
            draw_status("Simon Says!", ST77XX_WHITE, ST77XX_BLUE); 
            break;
        case STATE_SHOW_SEQUENCE: draw_status("Watch!", ST77XX_WHITE, ST77XX_BLUE); break;
        case STATE_WAIT_INPUT: draw_status("Your Turn!", ST77XX_BLACK, ST77XX_YELLOW); break;
        case STATE_GAME_OVER: {
            char str[128];
            sprintf(str, "HS: 1. %d, 2. %d, 3. %d", high_scores[0], high_scores[1], high_scores[2]);
            draw_screen(str); 
            draw_status("GAME OVER", ST77XX_WHITE, ST77XX_RED); 
            break;
        }
        case STATE_CORRECT: draw_status("CORRECT!", ST77XX_BLACK, ST77XX_GREEN); break;
    }
}

/**
 * @brief Reset game data, draw the game screen, and start level 1
*/
static void new_game(void) {
    leds_all_off();
    draw_screen("");
    for (uint8_t i = 0; i < HIST_MAX; i++) {
        int16_t x0 = (Display_width() - 4 * HIST_SP) / 2;
        int16_t x = x0 + i * HIST_SP;
        Display_drawCircle(x, HIST_Y, HIST_R + 1, ST77XX_WHITE);
    }
    seq_length = 0;
    seq_index = 0;
    led_lit = false;
    lit_color = -1;
    buzzerDisable(&buzzer1);
    add_random_color();
    last_tick = HAL_GetTick();
    change_state(STATE_SHOW_SEQUENCE);
}

/**
 * @brief START state: idle animation, SELECT starts countdown to game
 * 
 * @param press Button index from button_get_press(), NUM_COLORS is SELECT
*/
static void state_start(int8_t press) {
    static bool starting = false;
    static int8_t count = 4;

    if (press == NUM_COLORS) {
        starting = true;
        last_tick = HAL_GetTick();
        leds_all_off();
        draw_screen("Get Ready!");
        return;
    }

    if (starting) {
        uint32_t elapsed = HAL_GetTick() - last_tick;

        if (elapsed < 3000) {

            int8_t now = 3 - (elapsed / 1000);

            if (now != count) {
                count = now;
                char str[12];
                sprintf(str, "%d", 3 - (elapsed / 1000));
                draw_status(str, ST77XX_BLACK, ST77XX_YELLOW);
                if (count < 3) color_off(count);
                color_on(count - 1);
            }

        } else {
            starting = false;
            count = 4;
            color_off(0);
            new_game();
            change_state(STATE_SHOW_SEQUENCE);
        }

    } else if (HAL_GetTick() - last_tick >= FLASH_ON_MS) {

        color_off((uint8_t)lit_color);
        if (++lit_color >= NUM_COLORS) lit_color = 0;
        color_on((uint8_t)lit_color);
        last_tick = HAL_GetTick();

    }
}

/**
 * @brief GAME_OVER state: random LED animation until SELECT is pressed or 10s goes by
 * 
 * @param press Button index from button_get_press(), NUM_COLORS is SELECT
*/
static void state_game_over(int8_t press) {

    static uint8_t lit = 0;
    static uint32_t total = 0;

    if (press == 4 || total > 10000) {
        lit_color = 0;
        color_on(0);
        last_tick = HAL_GetTick();
        change_state(STATE_START);
        return;
    }

    uint32_t random;
    uint32_t elapsed = HAL_GetTick() - last_tick;
        
    if (elapsed >= 100) {
        total += elapsed;
        color_off(lit);
        if (HAL_RNG_GenerateRandomNumber(&hrng, &random) == HAL_OK) {
            lit = random % NUM_COLORS;
            color_on(lit);
        }
        last_tick = HAL_GetTick();
    }

}

/**
 * @brief SHOW_SEQUENCE state: play the sequence one color at a time.
 * 
 * Goes to WAIT_INPUT after the last color
*/
static void state_show_sequence(void) {
    uint32_t elapsed = HAL_GetTick() - last_tick;

    if (!led_lit && elapsed >= FLASH_OFF_MS) {
        color_on(sequence[seq_index]);
        led_lit = true;
        last_tick = HAL_GetTick();
    } else if (led_lit && elapsed >= FLASH_ON_MS) {
        color_off(sequence[seq_index]);
        led_lit = false;
        last_tick = HAL_GetTick();
        seq_index++;

        if (seq_index >= seq_length) {
            seq_index = 0;
            history_len = 0;
            change_state(STATE_WAIT_INPUT);
        }
    }
}

/**
 * @brief WAIT_INPUT state: record, display, and check each press
 * 
 * Adds each press to history[] and compared it with sequence[]. Mismatch
 * updates high scores and goes to GAME_OVER. A full correct sequence goes to CORRECT.
 * 
 * @param press Button index from button_get_press(), NUM_COLORS is SELECT, -1 is none
*/
static void state_wait_input(int8_t press) {
    if (lit_color != -1) {
        if (HAL_GetTick() - last_tick < PRESS_FLASH_MS) return;
        color_off(lit_color);
        lit_color = -1;

        if (seq_index >= seq_length) {
            if (seq_length >= MAX_SEQUENCE) {
                // No out of bounds exception
                change_state(STATE_GAME_OVER);
                return;
            }

            change_state(STATE_CORRECT);
        }
        return;
    }

    if (press == -1) return;

    history[history_len] = press;
    history_len++;

    draw_history();

    if (history[history_len - 1] != sequence[history_len - 1]) {
        int8_t score_index = -1;
        // Add score to high scores, if higher
        for (int i = 2; i >= 0; i--) {
            if (seq_length > high_scores[i]) score_index = i;
            else break;
        }
        if (score_index >= 0) high_scores[score_index] = seq_length;

        last_tick = HAL_GetTick();
        change_state(STATE_GAME_OVER);
        return;
    }

    color_on(press);
    lit_color = press;
    last_tick = HAL_GetTick();
    seq_index++;

}

/**
 * @brief CORRECT state: flash all colors, then add a color and continue.
*/
void state_correct(void) {
    static int count = 0;
    static bool lit = false;

    if (HAL_GetTick() - last_tick >= FLASH_OFF_MS) {

        for (int i = 0; i < NUM_COLORS; i++) {
            if (lit) color_off(i);
            else color_on(i);
        }

        lit = !lit;
        count++;
        last_tick = HAL_GetTick();

        if (count > 5) {
            leds_all_off();
            buzzerDisable(&buzzer1);
            add_random_color();
            seq_index = 0;
            led_lit = false;
            last_tick = HAL_GetTick();
            clear_history();
            count = 0;
            lit = false;
            change_state(STATE_SHOW_SEQUENCE);
        }

    }
}

void simon_init(void) {
    leds_all_off();
    buzzerDisable(&buzzer1);

    seq_length = 0;
    seq_index = 0;
    led_lit = false;
    lit_color = 0;
    color_on(0);
    last_tick = HAL_GetTick();
    change_state(STATE_START);
}

void simon_update(void) {

    int8_t press = button_get_press();

    if (state != STATE_START && press == NUM_COLORS) {
        leds_all_off();
        lit_color = 0;
        color_on(0);
        last_tick = HAL_GetTick();
        buzzerDisable(&buzzer1);
        change_state(STATE_START);
        return;
    }

    switch (state) {
        case STATE_START: state_start(press); break;
        case STATE_SHOW_SEQUENCE: state_show_sequence(); break;
        case STATE_GAME_OVER: state_game_over(press); break;
        case STATE_WAIT_INPUT: state_wait_input(press); break;
        case STATE_CORRECT: state_correct(); break;
    }
}