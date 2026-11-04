#include "stm32h5xx_hal.h"
#include "stdint.h"
#include "stdbool.h"

#define NOTE_A 440
#define NOTE_A_SHARP 474
#define NOTE_B_FLAT NOTE_A_SHARP
#define NOTE_B 494
#define NOTE_C 523
#define NOTE_C_SHARP 554
#define NOTE_D_FLAT NOTE_C_SHARP
#define NOTE_D 587
#define NOTE_D_SHARP 622
#define NOTE_E_FLAT NOTE_D_SHARP
#define NOTE_E 659
#define NOTE_F 698
#define NOTE_F_SHARP 740
#define NOTE_G_FLAT NOTE_F_SHARP
#define NOTE_G 784
#define NOTE_G_SHARP 831
#define NOTE_A_FLAT NOTE_G_SHARP

#define OCTAVE_UP(__NOTE__, __OCTAVE__) ((__NOTE__) * (__OCTAVE__))
#define OCTAVE_DOWN(__NOTE__, __OCTAVE__) ((__NOTE__) / (__OCTAVE__))

#define NUM_NOTES 12

typedef uint32_t Note;

typedef struct _buzzer {
    TIM_HandleTypeDef *buzzerTimerHandle;
    uint32_t buzzerChannel;
    bool currBuzzerState;
} Buzzer;

void buzzerEnable(Buzzer * const buzzer);
void buzzerDisable(Buzzer * const buzzer);

void buzzerSetNote(Buzzer const *buzzer, Note note, int32_t octave);

uint32_t buzzerGetPrescalerFromNote(Buzzer const *buzzer, Note note);