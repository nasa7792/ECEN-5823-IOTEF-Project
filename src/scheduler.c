/*
 * scheduler.c
 *
 *  Created on: Feb 4, 2026
 *      Author: Asus
 */

#include"scheduler.h"
volatile uint32_t schedulerEvents = 0;


void scheduler_setEvent_get_temp () {
CORE_DECLARE_IRQ_STATE;
// set event
CORE_ENTER_CRITICAL(); // enter critical, turn off interrupts in NVIC
schedulerEvents |= evtMeasureTemp; // RMW 0xb0011
CORE_EXIT_CRITICAL(); // exit critical, re-enable interrupts in NVIC
}

uint32_t getNextEvent(){
  uint32_t theEvent;
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  if(schedulerEvents&evtMeasureTemp){
      theEvent=evtMeasureTemp;
      schedulerEvents=schedulerEvents&~(evtMeasureTemp);
  }
  CORE_EXIT_CRITICAL();
  return theEvent;
}
