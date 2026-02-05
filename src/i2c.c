/*
 * i2c.c
 *
 *  Created on: Feb 1, 2026
 *      Author: Nalin Saxena
 * 
 * File Brief -Implementation file for I2C related apis. Contains function definitions related
 * to Si7021 communication
 */

#include"i2c.h"
#define INCLUDE_LOG_DEBUG 1
#include "log.h"
#include <string.h>

I2C_TransferReturn_TypeDef transferStatus; // make this global for IRQs in A4
I2C_TransferSeq_TypeDef transferSequence; // this one can be local
uint8_t cmd_data; // make this global for IRQs in A4
uint16_t read_data;
uint8_t buf[2];

//macros dervied from i2c example i2cspm_baremetal
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
 .i2cClhr = i2cClockHLRStandard
 };

void enable_Si7021(){
  GPIO_PinOutSet(TEMP_SENSOR_PORT, TEMP_SENSOR_ENABLE_PIN);

}
void disable_Si7021(){
  GPIO_PinOutClear(TEMP_SENSOR_PORT, TEMP_SENSOR_ENABLE_PIN);
}
//called during setup
void initialize_I2C0(){
  enable_Si7021();
  timerWaitUs(LOAD_PWR_MGMT_SENSOR);
  I2CSPM_Init(&I2C_Config);
}

void send_command_to_Si7021(){
  cmd_data = SI7021_CMD_MEASURE_TEMP_NO_HOLD;
  transferSequence.addr = SI7021_I2C_BUS_ADDRESS << 1; // shift device address left
  transferSequence.flags = I2C_FLAG_WRITE; //send write command
  transferSequence.buf[0].data = &cmd_data; // pointer to data to write
  transferSequence.buf[0].len = sizeof(cmd_data); //will contain 0xf3

  transferStatus = I2CSPM_Transfer (I2C0, &transferSequence);
  //check status of transfer done, and log error
  if (transferStatus != i2cTransferDone) {
   LOG_ERROR ("I2CSPM_Transfer: I2C bus write of cmd=0xF3 failed with status code of %d \n \r",transferStatus);
  }
  timerWaitUs(CONV_TIME);
}

void read_data_from_Si7021(){
  transferSequence.addr = SI7021_I2C_BUS_ADDRESS << 1; // shift device address left
  transferSequence.flags = I2C_FLAG_READ; //call with read address
  transferSequence.buf[0].data =  buf; // pointer to data to write
  transferSequence.buf[0].len = 2;

  transferStatus = I2CSPM_Transfer (I2C0, &transferSequence);
    //check status of transfer done, and log error
  if (transferStatus != i2cTransferDone) {
   LOG_ERROR ("I2CSPM_Transfer: I2C bus read failed with status code of %d \n \r",transferStatus);
  }
  read_data = ((uint16_t)buf[0] << 8) | buf[1];
  uint16_t temp_masked = read_data & MASK_TEMP;
  //macros for these values ask TA ??
  float temperature_c = ((175.72 * temp_masked) / 65536.0) - 46.85;
  //log temperature value
  LOG_INFO("Temperature value = %.2f °C \n \r", temperature_c);
}


void read_temp_from_si7021(){
  enable_Si7021();
  timerWaitUs(LOAD_PWR_MGMT_SENSOR); //80ms wait
  send_command_to_Si7021();
  read_data_from_Si7021();
  disable_Si7021(); //load power mgmt
}
