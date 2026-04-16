/*
 * HRSPO2_SENSOR.h
 *
 *  Created on: 15-Apr-2026
 *      Author: nalin
 *
 * A header file for hrspo2
 */

#ifndef SRC_HRSPO2_SENSOR_H_
#define SRC_HRSPO2_SENSOR_H_

#include"gpio.h"
#include"i2c.h"
#include"timer.h"

#define HRSPO2_RESET_HOLD_MS 10      // hold RESET low for 10ms
#define HRSPO2_BOOT_SETTLE_MS     50      // wait 50ms after releasing RESET
#define HRSPO2_INIT_TIME    1500      // wait 50ms after releasing RESET
#define HRSPO2_CMD_DELAY_MS       1       // min delay between write and read

#define CMD_DELAY 40

#define FAM_SET_OUTPUT_MODE      0x10
#define IDX_SET_OUTPUT_MODE      0x00
#define FAM_ENABLE_SENSOR        0x44
#define IDX_ENABLE_SENSOR        0x03 // Enable/Disable
#define FAM_ENABLE_ALGO          0x52
#define IDX_ENABLE_ALGO          0x02

#define SENSOR_ENABLE            0x01
#define ALGO_ENABLE              0x01
#define OUTPUT_ALGO_DATA         0x01 // 0x01 = Sensor + Algo data

extern uint8_t HRSPO2_rx_buf[10];
extern uint8_t HRSPO2_cmd_buf[3];

void initiliaze_HRSPO2_SENSOR(void);
void process_HRSPO2_values(void);
void read_fifo_from_HRSPO2(void);

#endif /* SRC_HRSPO2_SENSOR_H_ */
