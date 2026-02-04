/*
 * irq.c
 *
 *  Created on: Jan 25, 2026
 *      Author: Nalin Saxena
* File Brief- Implementation file for IRQ configuration apis and irq handler. Contains function def related
 * to IRQ enabling for LETIMER0 and irq hanlder which toggles the led
 */

#include"irq.h"

void enable_LETIMER0_interrupt(){
 // Enable LETIMER interrupt in NVIC
 NVIC_EnableIRQ(LETIMER0_IRQn);
}

void LETIMER0_IRQHandler(void)
{
  LETIMER_IntClear(LETIMER0, LETIMER_IF_UF);
  static int state=0;
  if(state==0)gpioLed1SetOn();
  else gpioLed1SetOff();
  state=!state;
}
