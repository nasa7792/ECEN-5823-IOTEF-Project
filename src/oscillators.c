/*
 * oscillators.c
 *
 *  Created on: Jan 28, 2026
 *      Author: Asus
 */

#include"oscillators.h"
void select_oscillator(void)
{
  if(LOWEST_ENERGY_MODE<=2){
      CMU_ClockEnable(cmuClock_LFA, true);
      CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
      CMU_ClockDivSet(cmuClock_LETIMER0, cmuClkDiv_4);
      CMU_ClockEnable(cmuClock_LETIMER0, true);
  }
}
