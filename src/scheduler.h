/*
 * scheduler.h
 *
 *  Created on: Feb 4, 2026
 *      Author: Asus
 */

#ifndef SRC_SCHEDULER_H_
#define SRC_SCHEDULER_H_
#include"em_core_generic.h"

typedef enum {
    evtMeasureTemp    = (1U << 0), // 0b0001
} event_list;

uint32_t getNextEvent();
void scheduler_setEvent_get_temp();
#endif /* SRC_SCHEDULER_H_ */
