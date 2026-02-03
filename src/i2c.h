/*
 * i2c.h
 *
 *  Created on: Feb 1, 2026
 *      Author: Asus
 */

#ifndef SRC_I2C_H_
#define SRC_I2C_H_

#include"sl_i2cspm.h"
#include"gpio.h"
#include"sl_i2cspm_sensor_config.h"

extern I2C_TransferReturn_TypeDef transferStatus; // make this global for IRQs in A4
extern I2C_TransferSeq_TypeDef transferSequence; // this one can be local
extern uint8_t cmd_data; // make this global for IRQs in A4
extern uint16_t read_data; // make this global for IRQs in A4
extern I2CSPM_Init_TypeDef I2C_Config;

#define SI7021_I2C_BUS_ADDRESS           0x40               /**< I2C bus address                        */
#define SI7021_CMD_MEASURE_TEMP_NO_HOLD  0xF3               /**< Measure Temperature, No Hold Master Mode */
#define SI7021_I2C_BUS_ADDRESS           0x40               /**< I2C bus address                        */

void initialize_I2C0();
void enable_Si7021();
void disable_Si7021();
void send_command_to_Si7021();
void read_data_from_Si7021();
void measure_temp_F3();
#endif /* SRC_I2C_H_ */
