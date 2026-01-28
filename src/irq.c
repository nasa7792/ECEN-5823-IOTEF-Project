/*
 * irq.c
 *
 *  Created on: Jan 25, 2026
 *      Author: Asus
 */

#include"irq.h"

void LETIMER0_IRQHandler(void)
{
    uint32_t flags = LETIMER_IntGet(LETIMER0);
    LETIMER_IntClear(LETIMER0, flags);
    static int state=1;

    if (flags & LETIMER_IF_COMP0)
    {
        if(state==1){
            gpioLed0SetOn();
        }
        else{
            gpioLed0SetOff();
        }
        state=!state;
    }
}
