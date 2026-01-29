/*
 * irq.c
 *
 *  Created on: Jan 25, 2026
 *      Author: Asus
 */

#include"irq.h"

void enable_LETIMER0_interrupt(){

}

void LETIMER0_IRQHandler(void)
{
    uint32_t flags = LETIMER_IntGet(LETIMER0);
    LETIMER_IntClear(LETIMER0, flags);
    if (flags & LETIMER_IF_COMP0)
    {
        gpioLed0SetOff();

    }
    if(flags & LETIMER_IF_COMP1)
    {
        gpioLed0SetOn();
    }
}
