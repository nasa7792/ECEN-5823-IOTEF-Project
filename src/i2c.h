/*
 * i2c.h
 *
 *  Created on: Feb 1, 2026
 *      Author: Nalin Saxena
 *
 * File Brief -Header file for I2C related apis. Contains function declarations related
 * to Si7021 communincation
 */

#ifndef SRC_I2C_H_
#define SRC_I2C_H_

#include "sl_i2cspm.h"
#include "gpio.h"
#include "em_core.h"
#include "string.h"
#include "em_i2c.h"
#include "timer.h"
#include "em_core_generic.h"
#include "lcd.h"
#include"HRSPO2_SENSOR.h"
#include "sl_i2cspm_sensor_config.h"

extern I2C_TransferReturn_TypeDef transferStatus; // make this global for IRQs in A4
extern I2C_TransferSeq_TypeDef transferSequence;  // this one can be local
extern uint8_t cmd_data;                          // make this global for IRQs in A4
extern uint16_t read_data;                        // make this global for IRQs in A4
extern I2CSPM_Init_TypeDef I2C_Config;

//macros dervied from i2c example i2cspm_baremetal
#define HRSPO2_I2C_BUS_ADDRESS 0x55          /**< I2C bus address                        */



/*
initliazies the i2c0 bus and enables communication with Si7021
*/
void initialize_I2C0();
void initialize_HRSPO2(void);
void write_config_register_HRSPO2(uint8_t family, uint8_t index, uint8_t value);
void read_spo2_sample();

void read_spo2_sample(void);
#endif /* SRC_I2C_H_ */
