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
#include "timer.h"
#include "em_core_generic.h"
#include "sl_i2cspm_sensor_config.h"

extern I2C_TransferReturn_TypeDef transferStatus; // make this global for IRQs in A4
extern I2C_TransferSeq_TypeDef transferSequence;  // this one can be local
extern uint8_t cmd_data;                          // make this global for IRQs in A4
extern uint16_t read_data;                        // make this global for IRQs in A4
extern I2CSPM_Init_TypeDef I2C_Config;

//macros dervied from i2c example i2cspm_baremetal
#define SI7021_I2C_BUS_ADDRESS 0x40          /**< I2C bus address                        */
#define SI7021_CMD_MEASURE_TEMP_NO_HOLD 0xF3 /**< Measure Temperature, No Hold Master Mode */
#define SI7021_I2C_BUS_ADDRESS 0x40          /**< I2C bus address                        */
#define MASK_TEMP 0XFFFC

/*
initliazies the i2c0 bus and enables communication with Si7021
*/
void initialize_I2C0();

/*
turns on the gpio enable assosicated with Si7021
*/
void enable_Si7021();
/*
turns off the gpio enable assosicated with Si7021
*/
void disable_Si7021();
/*
sends the temperature command 0xF3 to the Si7021 sensor
*/
void send_command_to_Si7021();
/*
reads the temperature data from the Si7021 sensor
*/
void read_data_from_Si7021();
/*
combined routine called from the schdeuler to enable the Si7021 sensor, send temp command,
wait for converison period, read data and log value, and finally disable the sensor
*/
void read_temp_from_si7021();
#endif /* SRC_I2C_H_ */
