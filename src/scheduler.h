/*
 * scheduler.h
 *
 *  Created on: Feb 4, 2026
 *      Author: Nalin Saxena
 * 
 * Edited on - 2/12/2026
 * 
 * File Brief -Header file for scheduler related apis. Contains function declarations related
 * to scheduler events
 */

#ifndef SRC_SCHEDULER_H_
#define SRC_SCHEDULER_H_
#include "em_core_generic.h"

typedef enum
{   //as per assignmnet documents we need to support 3 events
    evtLETIMER0_UnderFlow=(1U << 0), 
    evtLETIMER0_Comp1=(1U << 1),
    evtI2CTransferComplete=(1U << 2)
} event_list;


/*
checks the schedulerEvents variable to figure out which event should be addressed next
*/
uint32_t getNextEvent();

/*
called from LETimer0 irq and sets the evtLETIMER0_Comp1 event
*/
void scheduler_setEvent_UnderFlow();

/*
called from LETimer0 irq and sets the evtLETIMER0_UnderFlow event
*/
void scheduler_setEvent_COMP1();

/*
called from i2c event handler irq and sets the evtI2CTransferComplete event
*/
void scheduler_setEvent_I2C_Transfer_Complete();
#endif /* SRC_SCHEDULER_H_ */
