/*
 * timer.c
 *
 *  Created on: Jan 25, 2026
 *      Author: Asus
 */

#include "timer.h"

void letimer_init(void){

select_oscillator();
uint32_t REQ_TICKS_LED_PERIOD=(FEQ_LFXO*REQ_LED_PERIOD)/PRE_SCALER_LFXO;
uint32_t REQ_TICKS_LED_ON_TIME=(FEQ_LFXO*REQ_LED_ON_TIME)/PRE_SCALER_LFXO;
LETIMER_Init_TypeDef letimerInit = LETIMER_INIT_DEFAULT;

letimerInit.enable = false;      // Start later
letimerInit.comp0Top = true;     // Load COMP0 into CNT on underflow


LETIMER_Init(LETIMER0, &letimerInit);

// Set compare value 0 for 2.25 s
LETIMER_CompareSet(LETIMER0, 0, REQ_TICKS_LED_PERIOD);
LETIMER_IntClear(LETIMER0, LETIMER_IF_COMP0);

// Set compare value 1 for 175 ms
LETIMER_CompareSet(LETIMER0, 1, REQ_TICKS_LED_ON_TIME);
LETIMER_IntClear(LETIMER0, LETIMER_IF_COMP1);

 // Enable COMP0 interrupt
 LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP0);

 // Enable LETIMER interrupt in NVIC
 NVIC_EnableIRQ(LETIMER0_IRQn);

 // Start timer
 LETIMER_Enable(LETIMER0, true);
}

