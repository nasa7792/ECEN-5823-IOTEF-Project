/*
 * oscillators.h
 *
 *  Created on: Jan 28, 2026
 *      Author: Nalin Saxena
 *
 * File Brief- Header file for Oscillator configuration apis. Contains function declarations related
 * to oscillator selection based on energy mode
 */

#ifndef SRC_OSCILLATORS_H_
#define SRC_OSCILLATORS_H_

#include "app.h"
#include "em_cmu.h"

/*
Sets the Oscillator in use by the LETIMER0
uilizes the LOWEST_ENERGY_MODE to decide the osc
for EM0,EM1,EM2- we use LFX0
for EM3- we configure ULFRCO
*/
void select_oscillator(void);

#endif /* SRC_OSCILLATORS_H_ */
