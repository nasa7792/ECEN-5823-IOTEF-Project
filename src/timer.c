/*
 * timer.c
 *
 *  Created on: Jan 25, 2026
 *      Author: Nalin Saxena
 *
 * File Brief -Implementation file for LETIMER0 related apis. Contains function implementation related
 * to timer initlization and configuring interrupts at the correct frequency
 */

#include "timer.h"

void timerWaitUs(uint32_t us_wait){

  uint32_t wait_time_sec=us_wait/CONVERT_US_TO_SEC;
  uint32_t REQ_TICKS_WAIT=((FEQ_OSC * wait_time_sec) / (PRE_SCALER_OSC));

  uint32_t prev_tick_cnt = LETIMER_CounterGet(LETIMER0);
  uint32_t ticks_passed = 0;

  while(ticks_passed<REQ_TICKS_WAIT){
      uint32_t current_tick_cnt = LETIMER_CounterGet(LETIMER0);
      if(current_tick_cnt!=prev_tick_cnt){
          ticks_passed++;
          prev_tick_cnt=current_tick_cnt;
      }
  }

}

void letimer0_init(void)
{

    select_oscillator(); // based on the current EM configure our oscillator
    /*
    calculate the number of timer ticks for led period and the led on time
    */
    uint32_t REQ_TICKS_SAMPLING= ((FEQ_OSC * LETIMER_PERIOD_MS) / (PRE_SCALER_OSC))/CONVERT_MS_TO_SEC;
    LETIMER_Init_TypeDef letimerInit = LETIMER_INIT_DEFAULT;

    letimerInit.enable = false;  // Start timer later
    letimerInit.comp0Top = true; // Load COMP0 into CNT on underflow
    letimerInit.ufoa0      = letimerUFOANone;
    letimerInit.ufoa1      = letimerUFOANone;

    // make an init call to timer init
    LETIMER_Init(LETIMER0, &letimerInit);

    // Set compare value 0 for 3 s
    LETIMER_CompareSet(LETIMER0, 0, REQ_TICKS_SAMPLING);
    LETIMER_IntClear(LETIMER0, LETIMER_IF_UF);

    // Enable COMP0 and COMP1 interrupt
    //LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);

    //enable_LETIMER0_interrupt();

    // Start the timer
    LETIMER_Enable(LETIMER0, true);
}
