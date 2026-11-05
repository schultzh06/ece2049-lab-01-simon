/**
 * @file simon_io.c
 * @author Hayden Schultz
 * @brief Hardware layer for Simon Says game.
 * @date 2026-11-04
 * 
 * Configures GPIO pins through register writes, provides LED control 
 * and button reading for game logic
 * 
 * LED mappings
 *   LED1 (green): PC5; CN10-6
 *   LED2 (red): PB12; CN10-16
 *   LED3 (blue): PB13; CN10-30
 *   LED4 (yellow): PB8; CN10-36
 *
 * Button mappings
 *   BTN1 (green): PC11; CN7-2
 *   BTN2 (red): PC12; CN7-3
 *   BTN3 (blue): PC2; CN7-35
 *   BTN4 (yellow): PB0; CN7-34
 */

#include "simon_io.h"
#include "main.h"
#include "stm32h5xx_hal.h"
#include <stdint.h>

typedef struct { GPIO_TypeDef *port; uint16_t pin; } Pin;

static const Pin leds[] = {
    {GPIOC, GPIO_PIN_5}, {GPIOB, GPIO_PIN_12}, {GPIOB, GPIO_PIN_13}, {GPIOB, GPIO_PIN_8}
}, btns[] = {
    {GPIOC, GPIO_PIN_11}, {GPIOC, GPIO_PIN_12}, {GPIOC, GPIO_PIN_2}, {GPIOB, GPIO_PIN_0}
};

void simon_io_init(void) {

    // Enable GPIO port clocks
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOCEN;
    (void)RCC->AHB2ENR;

    // ---- Init registers for LEDs ----

    // MODER: 0b01 = Output mode
    GPIOC->MODER &= ~GPIO_MODER_MODE5; // Clear both bits
    GPIOC->MODER |= GPIO_MODER_MODE5_0; // Set bit 0
    GPIOB->MODER &= ~(GPIO_MODER_MODE12 | GPIO_MODER_MODE13 | GPIO_MODER_MODE8);
    GPIOB->MODER |= (GPIO_MODER_MODE12_0 | GPIO_MODER_MODE13_0 | GPIO_MODER_MODE8_0);

    // OTYPER: 0b0 = Push-pull
    GPIOC->OTYPER &= ~GPIO_OTYPER_OT5;
    GPIOB->OTYPER &= ~(GPIO_OTYPER_OT12 | GPIO_OTYPER_OT13 | GPIO_OTYPER_OT8);

    // OSPEEDR: 0b00 = low speed
    GPIOC->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED5;
    GPIOB->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED12 | GPIO_OSPEEDR_OSPEED13 | GPIO_OSPEEDR_OSPEED8);

    // PUPDR: 0b00 = no pull up or down
    GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD5;
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD12 | GPIO_PUPDR_PUPD13 | GPIO_PUPDR_PUPD8);

    // ---- Init registers for Buttons ----

    // MODER: 0b00 = Input
    GPIOC->MODER &= ~(GPIO_MODER_MODE11 | GPIO_MODER_MODE12 | GPIO_MODER_MODE2);
    GPIOB->MODER &= ~GPIO_MODER_MODE0;

    // PUPDR: 0b01 = pull up
    GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD11 | GPIO_PUPDR_PUPD12 | GPIO_PUPDR_PUPD2); // Clear
    GPIOC->PUPDR |= (GPIO_PUPDR_PUPD11_0 | GPIO_PUPDR_PUPD12_0 | GPIO_PUPDR_PUPD2_0); // Set bit 0
    GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD0;
    GPIOB->PUPDR |= GPIO_PUPDR_PUPD0_0; 

    for (int i = 0; i < 4; i++) {
        HAL_GPIO_WritePin(leds[i].port, leds[i].pin, GPIO_PIN_SET);
        HAL_Delay(300);
        HAL_GPIO_WritePin(leds[i].port, leds[i].pin, GPIO_PIN_RESET);
    }
}

void led_set(uint8_t i, bool on) {
    if (on) leds[i].port->BSRR = leds[i].pin; // Set low bit -> off
    else leds[i].port->BSRR = leds[i].pin << 16; // Set high bit -> on
}

void leds_all_off(void) {
    for (int i = 0; i < NUM_COLORS; i++) {
        led_set(i, false);
    }
}

int8_t button_get_press(void) {
    static int8_t held = -1;
    static uint32_t last_change = 0;

    int8_t current = -1;
    for (int i = 0; i < NUM_COLORS; i++) {
        if (!(btns[i].port->IDR & btns[i].pin)) {
            current = i;
            break;
        }
    }

    if (current == held) return -1;
    if (HAL_GetTick() - last_change < 20) return -1;

    last_change = HAL_GetTick();
    held = current;
    return current;
}