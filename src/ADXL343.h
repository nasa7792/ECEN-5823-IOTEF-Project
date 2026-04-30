/*
 * ADXL343.h
 *
 *  Created on: Apr 19, 2026
 *      Author: Abhirath Koushik
 */

#ifndef SRC_ADXL343_H_
#define SRC_ADXL343_H_

#include "i2c.h"

/* SDO pin tied to GND so 0x53 */
#define ADXL343_I2C_ADDR        0x53

/* Registers */
#define ADXL343_REG_DEVID       0x00
#define ADXL343_REG_POWER_CTL   0x2D
#define ADXL343_REG_DATA_FORMAT 0x31
#define ADXL343_REG_DATAX0      0x32

#define ADXL343_REG_THRESH_FF   0x28
#define ADXL343_REG_TIME_FF     0x29
#define ADXL343_REG_INT_ENABLE  0x2E
#define ADXL343_REG_INT_MAP     0x2F
#define ADXL343_REG_INT_SOURCE  0x30
#define INT_SRC_FREEFALL        (0x04)

/* INT1 pin on EXP header pin 6 */
#define ADXL343_INT_PORT        gpioPortC
#define ADXL343_INT_PIN         7

/* POWER_CTL bits */
#define ADXL343_MEASURE_MODE    0x08

extern uint8_t adxl_tx_buf[2];
extern uint8_t adxl_rx_buf[6];

void initialize_ADXL343(void);
void process_ADXL343_values(void);
uint8_t adxl_read_register(uint8_t reg);

#endif /* SRC_ADXL343_H_ */
