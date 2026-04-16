/*
 * i2c.c
 *
 *  Created on: Feb 1, 2026
 *      Author: Nalin Saxena
 *
 * File Brief -Implementation file for I2C related apis. Contains function definitions related
 * to Si7021 communication
 */

#include "i2c.h"
#define INCLUDE_LOG_DEBUG 1
#include "log.h"
#include <string.h>

I2C_TransferReturn_TypeDef transferStatus; // make this global for IRQs in A4
I2C_TransferSeq_TypeDef transferSequence;  // this one can be local

uint8_t HRSPO2_rx_buf[10];
uint8_t HRSPO2_cmd_buf[3];


// macros dervied from i2c example i2cspm_baremetal
I2CSPM_Init_TypeDef I2C_Config = {
    .port = I2C0,
    .sclPort = SL_I2CSPM_SENSOR_SCL_PORT,
    .sclPin = SL_I2CSPM_SENSOR_SCL_PIN,
    .sdaPort = SL_I2CSPM_SENSOR_SDA_PORT,
    .sdaPin = SL_I2CSPM_SENSOR_SDA_PIN,
    .portLocationScl = SL_I2CSPM_SENSOR_SCL_LOC,
    .portLocationSda = SL_I2CSPM_SENSOR_SDA_LOC,
    .i2cRefFreq = 0,
    .i2cMaxFreq = I2C_FREQ_STANDARD_MAX,
    .i2cClhr = i2cClockHLRStandard};


void initialize_I2C0()
{
  I2CSPM_Init(&I2C_Config);
}


void write_config_register_HRSPO2(uint8_t family, uint8_t index, uint8_t value)
{
  HRSPO2_cmd_buf[0]=family;
  HRSPO2_cmd_buf[1]=index;
  HRSPO2_cmd_buf[2]=value;


  transferSequence.addr = HRSPO2_I2C_BUS_ADDRESS << 1; // shift device address left
  transferSequence.flags = I2C_FLAG_WRITE;             // send write command
  transferSequence.buf[0].data = HRSPO2_cmd_buf;            // pointer to data to write
  transferSequence.buf[0].len = sizeof(HRSPO2_cmd_buf);

  NVIC_EnableIRQ(I2C0_IRQn);

  transferStatus = I2C_TransferInit(I2C0, &transferSequence);

  // check status of transfer done, and log error
  if (transferStatus < 0)
  {
    LOG_ERROR("I2CSPM_Transfer: I2C bus write of cmd failed with status code of %d \n \r", transferStatus);
  }

  while (I2C_Transfer(I2C0) == i2cTransferInProgress);
  timerWaitUs_polled(1000*HRSPO2_CMD_DELAY_MS);
}



