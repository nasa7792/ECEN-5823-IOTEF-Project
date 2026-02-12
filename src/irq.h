/*
 * irq.h
 *
 *  Created on: Jan 25, 2026
 *      Author: Nalin Saxena
 * File Brief- Header file for IRQ configuration apis. Contains function declarations related
 * to IRQ enabling for LETIMER0
 */

#ifndef SRC_IRQ_H_
#define SRC_IRQ_H_

#include"gpio.h"
#include "i2c.h"
#include "em_letimer.h"
#include"scheduler.h"


/*
call nvic functions to enable LETIMER0 INTERRUPT
*/
void enable_LETIMER0_interrupt();

/*
a timestamp helper function to count ms elapsed
*/
uint32_t letimerMilliseconds(void);
#endif /* SRC_IRQ_H_ */
