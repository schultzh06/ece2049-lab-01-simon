#include "buzzer.h"
#include "stm32h5xx_hal.h"
#include "stm32h5xx_hal_tim.h"
#include "stdbool.h"

Note const chromaticScale[NUM_NOTES] = {NOTE_A, NOTE_A_SHARP, NOTE_B, NOTE_C, NOTE_C_SHARP, NOTE_D, NOTE_D_SHARP, NOTE_E, NOTE_F, NOTE_F_SHARP, NOTE_G, NOTE_G_SHARP};

void buzzerEnable(Buzzer * const buzzer) {
    HAL_TIM_PWM_Start(buzzer->buzzerTimerHandle, buzzer->buzzerChannel);
    buzzer->currBuzzerState = true;
}

void buzzerDisable(Buzzer * const buzzer) {
    HAL_TIM_PWM_Stop(buzzer->buzzerTimerHandle, buzzer->buzzerChannel);
    buzzer->currBuzzerState = false;
}

void buzzerSetNote(Buzzer const *buzzer, Note note, int32_t octave) {
    Note adjustedNote = note;
    if (octave > 1) adjustedNote = OCTAVE_UP(note, octave);
    else if (octave <= 0) adjustedNote = OCTAVE_DOWN(note, -(octave - 2));

    __HAL_TIM_SET_PRESCALER(buzzer->buzzerTimerHandle, buzzerGetPrescalerFromNote(buzzer, adjustedNote));
}

uint32_t buzzerGetPrescalerFromNote(Buzzer const *buzzer, Note note) {
    return HAL_RCC_GetHCLKFreq() / (__HAL_TIM_GET_AUTORELOAD(buzzer->buzzerTimerHandle) * note);
}