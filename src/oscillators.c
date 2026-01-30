/*
 * oscillators.c
 *
 *  Created on: Jan 28, 2026
 *      Author: Nalin Saxena
 *
 * File Brief- Impementation file for Oscillator configuration apis. Contains function definition to related
 * to oscillator selection based on energy mode
 */

#include "oscillators.h"
void select_oscillator(void)
{
    /*below two instructions are common accross energy modes*/
    CMU_ClockEnable(cmuClock_LFA, true);
    CMU_ClockDivSet(cmuClock_LETIMER0, cmuClkDiv_4);
    // we need to correctly choose our oscillator
    if (LOWEST_ENERGY_MODE <= EM2_MODE)
    {
        CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
    }
    // we are now in the EM3 mode
    else
    {
        CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);
    }
    CMU_ClockEnable(cmuClock_LETIMER0, true);
}
