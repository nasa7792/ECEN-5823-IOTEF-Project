/*
 * scheduler.h
 *
 *  Created on: Feb 4, 2026
 *      Author: Nalin Saxena
 * 
 * Edited on - 3/25/2026
 * 
 * File Brief -Header file for scheduler related apis. Contains function declarations related
 * to scheduler events
 */

#ifndef SRC_SCHEDULER_H_
#define SRC_SCHEDULER_H_
#include "em_core_generic.h"
#include "sl_bt_api.h"

typedef enum
{   //more events to be supported
    evtLETIMER0_UnderFlow=(1U << 0), 
    evtLETIMER0_Comp1=(1U << 1),
    evtI2CTransferComplete=(1U << 2),
    //new events
    evtPB0Pressed=(1U<<3),
    evtPB0Released=(1U<<4),
    evtPB1Pressed=(1U<<5),
    evtPB1Released=(1U<<6),
} event_list;


/*
checks the schedulerEvents variable to figure out which event should be addressed next
*/
uint32_t getNextEvent();

/*
scheduler_setEvent_XXX() will now be handled via the sl_bt_external_signal calls
*/

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

/*
 sets the pb1 pressed event
*/
void setEvent_PB0_Pressed();


/*
 sets the pb0 released event
*/
void setEvent_PB0_Released();


/*
 sets the pb1 pressed event
*/
void setEvent_PB1_Pressed();


/*
sets the pb1 released event
*/
void setEvent_PB1_Released();


/*
a modified version of the state machine from a4, 
the state machine is now driven by ble events
*/
void temperature_state_machine (sl_bt_msg_t *evt);

/*
a new! state machine for the client board, which handles discovery states
*/
void discovery_state_machine(sl_bt_msg_t *evt);
#endif /* SRC_SCHEDULER_H_ */

