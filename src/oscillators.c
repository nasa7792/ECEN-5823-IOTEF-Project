/*
 * oscillators.c
 *
 *  Created on: Jan 28, 2026
 *      Author: Asus
 */

#include"oscillators.h"
void select_oscillator(void)
{
  CMU_ClockEnable(cmuClock_LFA, true);
  CMU_ClockDivSet(cmuClock_LETIMER0, cmuClkDiv_4);

  if(LOWEST_ENERGY_MODE<=2){
      CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
  }
  //we are now in the EM3 mode
  else{
      CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);
  }
  CMU_ClockEnable(cmuClock_LETIMER0, true);
}
