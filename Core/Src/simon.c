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

#define MAX_SEQUENCE 100
#define FLASH_ON_MS 500
#define FLASH_OFF_MS 200
#define PRESS_FLASH_MS 300

#define TEXT_SIZE 2
#define CHAR_W (6 * TEXT_SIZE)
#define CHAR_H (8 * TEXT_SIZE)
#define CIRCLE_R 10
#define CIRCLE_Y (Display_height() - CIRCLE_R - 4)

#define HIST_R 7
#define HIST_SP 18
#define HIST_Y 119
#define HIST_MAX 5

#define BAR_H (Display_height() / 5)
#define BAR_PAD 4

extern Buzzer buzzer1;
extern RNG_HandleTypeDef hrng;

typedef enum {
    STATE_START,
    STATE_SHOW_SEQUENCE,
    STATE_WAIT_INPUT,
    STATE_GAME_OVER,
    STATE_CORRECT
} SimonState;

static SimonState state = STATE_START;
static uint8_t sequence[MAX_SEQUENCE];
static uint8_t seq_length = 0;
static uint8_t seq_index = 0;
static uint8_t history[MAX_SEQUENCE];
static uint8_t history_len = 0;
static bool led_lit = false;
static uint32_t last_tick;
static int8_t lit_color = -1;

static uint8_t high_scores[3] = {0, 0, 0};

static const Note color_notes[NUM_COLORS] = {NOTE_C, NOTE_E, NOTE_G, NOTE_A};
static const uint16_t circle_colors[NUM_COLORS] = {ST77XX_GREEN, ST77XX_RED, ST77XX_BLUE, ST77XX_YELLOW};

static void color_on(uint8_t i) {
    led_set(i, true);
    Display_fillRoundRect(42 + (i > 0 && i < 3 ? 40 : 0), (i > 1 ? 68 : 28), 36, 36, 5, circle_colors[i]);
    buzzerSetNote(&buzzer1, color_notes[i], 1);
    buzzerEnable(&buzzer1);
}

static void color_off(uint8_t i) {
    led_set(i, false);
    Display_fillRoundRect(42 + (i > 0 && i < 3 ? 40 : 0), (i > 1 ? 68 : 28), 36, 36, 5, ST77XX_WHITE);
    buzzerDisable(&buzzer1);
}

static void add_random_color(void) {
    uint32_t random;
    if (HAL_RNG_GenerateRandomNumber(&hrng, &random) == HAL_OK) {
        sequence[seq_length] = random % NUM_COLORS;
        seq_length++;
    }
}

static void draw_history(void) {
    uint8_t start = (history_len > 5) ? history_len - 5 : 0;
    for (uint8_t i = start; i < history_len; i++) {
        int16_t x0 = (Display_width() - 4 * HIST_SP) / 2;
        int16_t x = x0 + (i - start) * HIST_SP;
        Display_fillCircle(x, HIST_Y, HIST_R, circle_colors[history[i]]);
    }
}

static void clear_history(void) {
    for (uint8_t i = 0; i < HIST_MAX; i++) {
        int16_t x0 = (Display_width() - 4 * HIST_SP) / 2;
        int16_t x = x0 + i * HIST_SP;
        Display_fillCircle(x, HIST_Y, HIST_R, ST77XX_BLACK);
    }
}

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

static void draw_screen(char* msg) {
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

static void state_game_over(int8_t press) {

    static uint8_t lit = 0;

    if (press == 4) {
        lit_color = 0;
        color_on(0);
        last_tick = HAL_GetTick();
        change_state(STATE_START);
        return;
    }

    uint32_t random;
    uint32_t elapsed = HAL_GetTick() - last_tick;
        
    if (elapsed >= 100) {
        color_off(lit);
        if (HAL_RNG_GenerateRandomNumber(&hrng, &random) == HAL_OK) {
            lit = random % NUM_COLORS;
            color_on(lit);
        }
        last_tick = HAL_GetTick();
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
            history_len = 0;
            change_state(STATE_WAIT_INPUT);
        }
    }
}

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

        change_state(STATE_GAME_OVER);
        return;
    }

    color_on(press);
    lit_color = press;
    last_tick = HAL_GetTick();
    seq_index++;

}

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