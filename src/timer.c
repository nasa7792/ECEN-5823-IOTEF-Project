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

void letimer0_init(void)
{

    select_oscillator(); // based on the current EM configure our oscillator
    /*
    calculate the number of timer ticks for led period and the led on time
    */
    uint32_t REQ_TICKS_LED_PERIOD = ((FEQ_OSC * LETIMER_PERIOD_MS) / (PRE_SCALER_OSC))/CONVERT_MS_TO_SEC;
    uint32_t REQ_TICKS_LED_ON_TIME = ((FEQ_OSC * LETIMER_ON_TIME_MS) / (PRE_SCALER_OSC))/CONVERT_MS_TO_SEC;
    LETIMER_Init_TypeDef letimerInit = LETIMER_INIT_DEFAULT;

    letimerInit.enable = false;  // Start later
    letimerInit.comp0Top = true; // Load COMP0 into CNT on underflow

    // make an init call to timer init
    LETIMER_Init(LETIMER0, &letimerInit);

    // Set compare value 0 for 2.25 s
    LETIMER_CompareSet(LETIMER0, 0, REQ_TICKS_LED_PERIOD);
    LETIMER_IntClear(LETIMER0, LETIMER_IF_COMP0);

    // Set compare value 1 for 175 ms
    LETIMER_CompareSet(LETIMER0, 1, REQ_TICKS_LED_ON_TIME);
    LETIMER_IntClear(LETIMER0, LETIMER_IF_COMP1);

    // Enable COMP0 and COMP1 interrupt
    LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP0 | LETIMER_IEN_COMP1);

    enable_LETIMER0_interrupt();

    // Start the timer
    LETIMER_Enable(LETIMER0, true);
}
