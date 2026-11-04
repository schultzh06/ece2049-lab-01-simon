/**
* Functions for using the DWT (Data Watchpoint and Trace) unit to create accurate delays.
* Useful for applications requiring stricter time requirements and accuracy than HAL_Delay
* 
* Functions adapted from https://deepbluembedded.com/stm32-delay-microsecond-millisecond-utility-dwt-delay-timer-delay/
* by Deep Blue Embedded, with modifications for use in ECE 2049 by Kyle Schmottlach and Shannon Miranda '26
*/

#ifndef DWT_MODULE_H
#define DWT_MODULE_H

#include "stdint.h"
#include "stm32h5xx_nucleo.h"
#include "stm32h5xx_hal.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Initialization function for the Data Watchpoint Trigger (DWT) timer unit. This timer allows for accurate microsecond (us) and millisecond (ms)
 *        delays, providing better accuracy and performance than the HAL_Delay function.
 * 
 * @return true if the DWT timer was initialized successfully
 * @return false if the DWT timer failed to initialize
 */
bool DWT_Init(void);

/**
 * @brief Helper function to check whether the DWT timer has been initialized. Used to prevent errors in usage when it is unknown whether DWT_Init
 *        has been called.
 * 
 * @return true if the DWT is initialized and ready to use
 * @return false if the DWT is not initialized
 */
bool DWT_CheckInitialized(void);

/**
 * @brief Delays execution for a specified number of microseconds using the DWT timer. This function is blocking using CPU polling
 * @note This function requires that DWT_Init() has been called and returned successfully, otherwise the function will block indefinitely
 * @note This function is inline such that there is not extra overhead from function calls and context switching, allowing for more accurate timing
 * 
 * @param us The number of microseconds to delay execution for
 */
inline void DWT_delayUs(volatile uint32_t us) {
    uint32_t startTick = DWT->CYCCNT;
    uint32_t delayTicks = us * (HAL_RCC_GetHCLKFreq() / 1000000);
    while ((uint32_t)(DWT->CYCCNT - startTick) < delayTicks);
}

/**
 * @brief Delays execution for a specified number of milliseconds using the DWT timer. This function is blocking using CPU polling
 * @note This function requires that DWT_Init() has been called and returned successfully, otherwise the function will block indefinitely
 * @note This function is inline such that there is not extra overhead from function calls and context switching, allowing for more accurate timing
 * 
 * @param ms The number of milliseconds to delay execution for
 */
inline void DWT_delayMs(volatile uint32_t ms) {
    uint32_t startTick = DWT->CYCCNT;
    uint32_t delayTicks = ms * (HAL_RCC_GetHCLKFreq() / 1000);
    while ((uint32_t)(DWT->CYCCNT - startTick) < delayTicks);
}

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // DWT_MODULE_H