/*
 * timer.h
 *
 *  Created on: Jan 25, 2026
 *      Author: Nalin Saxena
 *
 * File Brief -Header file for LETIMER0 related apis. Contains function declarations related
 * to timer initlization
 */

#ifndef SRC_TIMER_H_
#define SRC_TIMER_H_

#include "em_cmu.h"
#include "em_letimer.h"
#include "em_core.h"
#include "oscillators.h"
#include "app.h"
#include "irq.h"
#define LOAD_PWR_MGMT_SENSOR (80000)//80ms max time to turn on temp Si7021
#define CONV_TIME (10500) // as per data sheet 10.5 is conversion time for sensor
#define MAX_US_SUPPORTED ((uint32_t)((0xFFFF) /  (FEQ_OSC / PRE_SCALER_OSC))*CONVERT_US_TO_SEC)
#define MIN_US_SUPPORTED ((uint32_t)((CONVERT_US_TO_SEC) /  (FEQ_OSC / PRE_SCALER_OSC)))


/*
Function to configure LETIMER0 to generate interrupts at the user specified rate 
in this case 2.25s and blink an LED with a on time of 175ms
*/
void letimer0_init(void);
void timerWaitUs(uint32_t us_wait);
#endif /* SRC_TIMER_H_ */
