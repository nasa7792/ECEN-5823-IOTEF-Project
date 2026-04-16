/*
 * timer.c
 *
 *  Created on: Jan 25, 2026
 *      Author: Nalin Saxena
 * 
 * Edited on - 2/12/2026
 *
 * File Brief -Implementation file for LETIMER0 related apis. Contains function implementation related
 * to timer initlization and configuring interrupts at the correct frequency
 */

#include "timer.h"
#include"sl_power_manager.h"


void timerWaitUs_polled(uint32_t us_wait)
{

  uint32_t letimer_freq = FEQ_OSC / PRE_SCALER_OSC;
  // range check for a value which is over what we can support
  // clamp down value if crossing above bounds
  if (us_wait > MAX_US_SUPPORTED)
  {
    us_wait = MAX_US_SUPPORTED;
  }
  // range check for a value which is below what we can support
  // clamp up value if below bounds
  if (us_wait < MIN_US_SUPPORTED)
  {
    us_wait = MIN_US_SUPPORTED;
  }

  uint32_t REQ_TICKS_WAIT =
      (letimer_freq * us_wait + (CONVERT_US_TO_SEC - 1)) / CONVERT_US_TO_SEC;

  uint32_t prev_cnt = LETIMER_CounterGet(LETIMER0);
  uint32_t ticks_passed = 0;

  while (ticks_passed < REQ_TICKS_WAIT)
  {
    uint32_t curr_cnt = LETIMER_CounterGet(LETIMER0);

    if (curr_cnt <= prev_cnt)
    {
      ticks_passed += (prev_cnt - curr_cnt);
    }
    else
    {
      // underflow occured went to 0
      ticks_passed += prev_cnt;
    }

    prev_cnt = curr_cnt;
  }
}


void timerWaitUs_irq(uint32_t us_wait)
{
  uint32_t letimer_freq = FEQ_OSC / PRE_SCALER_OSC;

  if (us_wait > MAX_US_SUPPORTED) {
    us_wait = MAX_US_SUPPORTED;
  }
  if (us_wait < MIN_US_SUPPORTED) {
    us_wait = MIN_US_SUPPORTED;
  }

  uint32_t REQ_TICKS_WAIT =
      (letimer_freq * us_wait + (CONVERT_US_TO_SEC - 1)) / CONVERT_US_TO_SEC;

  uint32_t curr_cnt = LETIMER_CounterGet(LETIMER0);
  uint32_t comp0_val = LETIMER_CompareGet(LETIMER0, 0);
  uint32_t comp1_val;

  if (curr_cnt >= REQ_TICKS_WAIT) {
      comp1_val = curr_cnt - REQ_TICKS_WAIT;
  } else {
      // in case pf wrap around
      comp1_val = comp0_val - (REQ_TICKS_WAIT - curr_cnt);
  }


  LETIMER_IntClear(LETIMER0, LETIMER_IF_COMP1);

  LETIMER_CompareSet(LETIMER0, 1, comp1_val);

  // Enable COMP1 interrupt generation
  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1);
}

void letimer0_init(void)
{

  select_oscillator(); // based on the current EM configure our oscillator
  /*
  calculate the number of timer ticks for requested period
  */
  uint32_t REQ_TICKS_SAMPLING = ((FEQ_OSC * LETIMER_PERIOD_MS) / (PRE_SCALER_OSC)) / CONVERT_MS_TO_SEC;
  LETIMER_Init_TypeDef letimerInit = LETIMER_INIT_DEFAULT;

  letimerInit.enable = false;  // Start timer later
  letimerInit.comp0Top = true; // Load COMP0 into CNT on underflow
  letimerInit.ufoa0 = letimerUFOANone;
  letimerInit.ufoa1 = letimerUFOANone;

  // make an init call to timer init
  LETIMER_Init(LETIMER0, &letimerInit);

  // Set compare value 0 for 5 s
  LETIMER_CompareSet(LETIMER0, 0, REQ_TICKS_SAMPLING);
  LETIMER_IntClear(LETIMER0, LETIMER_IF_UF | LETIMER_IF_COMP1);

  //enable uf interrupt this will be the hearbeat of the fsm, we will enable comp1 via interrupt
  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);

  enable_LETIMER0_interrupt();

  // Start the timer
  LETIMER_Enable(LETIMER0, true);
}
