/**
* Functions for using the DWT (Data Watchpoint and Trace) unit to create accurate delays.
* Useful for applications requiring stricter time requirements and accuracy than HAL_Delay
* 
* Functions adapted from https://deepbluembedded.com/stm32-delay-microsecond-millisecond-utility-dwt-delay-timer-delay/
* by Deep Blue Embedded, with modifications for use in ECE 2049 by Kyle Schmottlach and Shannon Miranda '26
*/

#include "dwt_module.h"

bool DWT_Init() {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // Enable trace
    DWT->CYCCNT = 0; // Reset the cycle counter
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; // Enable the cycle counter
    
    // Wait a few clock cycles to let the counter increase. Used to check whether initialization was successful
    __asm volatile ("nop");
    __asm volatile ("nop");
    __asm volatile ("nop");

    return DWT->CYCCNT != 0;
}

bool DWT_CheckInitialized() {
    return (CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk) && (DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk);
}