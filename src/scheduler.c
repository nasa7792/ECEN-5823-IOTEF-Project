/*
 * scheduler.c
 *
 *  Created on: Feb 4, 2026
 *      Author: Nalin Saxena
 *
 *  Edited on - 2/12/2026
 *
 * File Brief -implementation file for scheduler related apis. Contains function defs related
 * to scheduler events
 */

#include "scheduler.h"
#define INCLUDE_LOG_DEBUG 1
#include "timer.h"
#include "log.h"
volatile uint32_t schedulerEvents = 0;

void scheduler_setEvent_UnderFlow()
{
  CORE_DECLARE_IRQ_STATE;
  // set event
  CORE_ENTER_CRITICAL(); // enter critical, turn off interrupts in NVIC
  schedulerEvents |= evtLETIMER0_UnderFlow;
  CORE_EXIT_CRITICAL(); // exit critical, re-enable interrupts in NVIC
}

void scheduler_setEvent_COMP1()
{
  CORE_DECLARE_IRQ_STATE;
  // set event
  CORE_ENTER_CRITICAL(); // enter critical, turn off interrupts in NVIC
  schedulerEvents |= evtLETIMER0_Comp1;
  CORE_EXIT_CRITICAL(); // exit critical, re-enable interrupts in NVIC
}

void scheduler_setEvent_I2C_Transfer_Complete()
{
  CORE_DECLARE_IRQ_STATE;
  // set event
  CORE_ENTER_CRITICAL(); // enter critical, turn off interrupts in NVIC
  schedulerEvents |= evtI2CTransferComplete;
  CORE_EXIT_CRITICAL(); // exit critical, re-enable interrupts in NVIC
}

uint32_t getNextEvent()
{
  uint32_t theEvent = 0; // Use 0 for "no event or default"
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  // why does this ordering effect the code execution ??? 
  if (schedulerEvents & evtI2CTransferComplete)
  {
    theEvent = evtI2CTransferComplete;
    schedulerEvents &= ~evtI2CTransferComplete;
  }

  else if (schedulerEvents & evtLETIMER0_Comp1)
  {
    theEvent = evtLETIMER0_Comp1;
    schedulerEvents &= ~evtLETIMER0_Comp1;
  }

  else if (schedulerEvents & evtLETIMER0_UnderFlow)
  {
    theEvent = evtLETIMER0_UnderFlow;
    schedulerEvents &= ~evtLETIMER0_UnderFlow;
  }

  CORE_EXIT_CRITICAL();
  return theEvent;
}
