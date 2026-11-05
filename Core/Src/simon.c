#include "simon.h"
#include "Adafruit_ST77xx_Macros.h"
#include "main.h"
#include "buzzer.h"
#include "simon_io.h"
#include "stm32h5xx_hal.h"
#include "stm32h5xx_hal_rng.h"
#include <stdint.h>
#include <stdbool.h>
#include "Adafruit_ST7735_API.h"
#include "Adafruit_ST7735_Macros.h"
#include <string.h>

#define MAX_SEQUENCE 100
#define FLASH_ON_MS 500
#define FLASH_OFF_MS 200
#define PRESS_FLASH_MS 300

#define TEXT_SIZE 2
#define CHAR_W (6 * TEXT_SIZE)
#define CHAR_H (8 * TEXT_SIZE)

extern Buzzer buzzer1;
extern RNG_HandleTypeDef hrng;

typedef enum {
    STATE_START,
    STATE_SHOW_SEQUENCE,
    STATE_WAIT_INPUT,
    STATE_GAME_OVER
} SimonState;

static SimonState state = STATE_START;
static uint8_t sequence[MAX_SEQUENCE];
static uint8_t seq_length = 0;
static uint8_t seq_index = 0;
static bool led_lit = false;
static uint32_t last_tick;
static int8_t lit_color = -1;

static const Note color_notes[NUM_COLORS] = {NOTE_C, NOTE_E, NOTE_G, NOTE_A};

static void color_on(uint8_t i) {
    led_set(i, true);
    buzzerSetNote(&buzzer1, color_notes[i], 1);
    buzzerEnable(&buzzer1);
}

static void color_off(uint8_t i) {
    led_set(i, false);
    buzzerDisable(&buzzer1);
}

static void add_random_color(void) {
    uint32_t random;
    if (HAL_RNG_GenerateRandomNumber(&hrng, &random) == HAL_OK) {
        sequence[seq_length] = random % NUM_COLORS;
        seq_length++;
    }
}

static void draw_screen(const char *text, uint16_t text_color, uint16_t bg_color) {
    Display_fillScreen(bg_color);
    Display_setTextSize(TEXT_SIZE, TEXT_SIZE);
    Display_setTextColor(text_color);
    Display_setCursor((Display_width() - (int16_t)strlen(text) * CHAR_W) / 2, (Display_height() - CHAR_H) / 2);
    Display_print(text);
}

static void change_state(SimonState next) {
    state = next;

    switch(next) {
        case STATE_START: draw_screen("Press Start", ST77XX_WHITE, ST7735_BLACK); break;
        case STATE_SHOW_SEQUENCE: draw_screen("Watch...", ST77XX_BLACK, ST7735_YELLOW); break;
        case STATE_WAIT_INPUT: draw_screen("Your Turn!", ST77XX_BLACK, ST7735_GREEN); break;
        case STATE_GAME_OVER: draw_screen("Game Over.", ST77XX_WHITE, ST7735_RED); break;
    }
}

static void new_game(void) {
    leds_all_off();
    seq_length = 0;
    seq_index = 0;
    led_lit = false;
    lit_color = -1;
    buzzerDisable(&buzzer1);
    add_random_color();
    last_tick = HAL_GetTick();
    change_state(STATE_SHOW_SEQUENCE);
}

static void state_start(void) {
    int8_t press = button_get_press();
    if (press != -1) {
        new_game();
        return;
    }

    if (HAL_GetTick() - last_tick >= FLASH_ON_MS) {
        color_off((uint8_t)lit_color);
        if (++lit_color >= NUM_COLORS) lit_color = 0;
        color_on((uint8_t)lit_color);
        last_tick = HAL_GetTick();
    }
}

static void state_game_over(void) {
    if (button_get_press() != -1) {
        lit_color = 0;
        color_on(0);
        last_tick = HAL_GetTick();
        change_state(STATE_START);
    }
}

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
            change_state(STATE_WAIT_INPUT);
        }
    }
}

static void state_wait_input(void) {
    if (lit_color != -1) {
        if (HAL_GetTick() - last_tick < PRESS_FLASH_MS) return;
        color_off(lit_color);
        lit_color = -1;

        if (seq_index >= seq_length) {
            if (seq_length >= MAX_SEQUENCE) {
                // No out of bounds exception
                // TODO win?
                change_state(STATE_GAME_OVER);
                return;
            }
            add_random_color();
            seq_index = 0;
            led_lit = false;
            last_tick = HAL_GetTick();
            change_state(STATE_SHOW_SEQUENCE);
        }
        return;
    }

    int8_t press = button_get_press();
    if (press == -1) return;

    if (press != sequence[seq_index]) {
        change_state(STATE_GAME_OVER);
        return;
    }

    color_on(press);
    lit_color = press;
    last_tick = HAL_GetTick();
    seq_index++;

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
    switch (state) {
        case STATE_START: state_start(); break;
        case STATE_SHOW_SEQUENCE: state_show_sequence(); break;
        case STATE_GAME_OVER: state_game_over(); break;
        case STATE_WAIT_INPUT: state_wait_input(); break;
    }
}