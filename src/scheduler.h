/*
 * scheduler.h
 *
 *  Created on: Feb 4, 2026
 *      Author: Nalin Saxena
 * File Brief -Header file for scheduler related apis. Contains function declarations related
 * to scheduler events
 */

#ifndef SRC_SCHEDULER_H_
#define SRC_SCHEDULER_H_
#include "em_core_generic.h"

typedef enum
{
    evtMeasureTemp = (1U << 0), // 0b0001
} event_list;

/*
checks the schedulerEvents variable to figure out which event should be addressed next
*/
uint32_t getNextEvent();
/*
called from timer0 irq and sets the evtMeasureTemp event
*/
void scheduler_setEvent_get_temp();
#endif /* SRC_SCHEDULER_H_ */
