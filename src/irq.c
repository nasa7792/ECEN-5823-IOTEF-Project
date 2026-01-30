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
    uint32_t flags = LETIMER_IntGet(LETIMER0);
    //clear interrupts
    LETIMER_IntClear(LETIMER0, flags);
    //address interrupts
    if (flags & LETIMER_IF_COMP0)
    {
        gpioLed0SetOff();

    }
    if(flags & LETIMER_IF_COMP1)
    {
        gpioLed0SetOn();
    }
}
