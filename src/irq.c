/*
 * irq.c
 *
 *  Created on: Jan 25, 2026
 *      Author: Nalin Saxena
* File Brief- Implementation file for IRQ configuration apis and irq handler. Contains function def related
 * to IRQ enabling for LETIMER0 and irq hanlder which calls for a temp reading
 */

#include"irq.h"

void enable_LETIMER0_interrupt(){
 // Enable LETIMER interrupt in NVIC
 NVIC_EnableIRQ(LETIMER0_IRQn);
}

void LETIMER0_IRQHandler(void)
{
  LETIMER_IntClear(LETIMER0, LETIMER_IF_UF); //clear interrupt
  scheduler_setEvent_get_temp(); //get a temp reading
}
