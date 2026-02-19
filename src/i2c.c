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
uint8_t cmd_data;                          // make this global for IRQs in A4
uint16_t read_data;
uint8_t buf[2];

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

void enable_Si7021()
{
  GPIO_PinOutSet(TEMP_SENSOR_PORT, TEMP_SENSOR_ENABLE_PIN);
}
void disable_Si7021()
{
  GPIO_PinOutClear(TEMP_SENSOR_PORT, TEMP_SENSOR_ENABLE_PIN);
}
// called during setup
void initialize_I2C0()
{
  // enable_Si7021();
  // timerWaitUs_polled(LOAD_PWR_MGMT_SENSOR); // no polling allowed
  I2CSPM_Init(&I2C_Config);
}

void send_command_to_Si7021()
{
  cmd_data = SI7021_CMD_MEASURE_TEMP_NO_HOLD;
  transferSequence.addr = SI7021_I2C_BUS_ADDRESS << 1; // shift device address left
  transferSequence.flags = I2C_FLAG_WRITE;             // send write command
  transferSequence.buf[0].data = &cmd_data;            // pointer to data to write
  transferSequence.buf[0].len = sizeof(cmd_data);      // will contain 0xf3

  NVIC_EnableIRQ(I2C0_IRQn);

  transferStatus = I2C_TransferInit(I2C0, &transferSequence);
  // check status of transfer done, and log error
  if (transferStatus < 0)
  {
    LOG_ERROR("I2CSPM_Transfer: I2C bus write of cmd=0xF3 failed with status code of %d \n \r", transferStatus);
  }
}

void process_temperature_reading()
{
  //
  read_data = ((uint16_t)buf[0] << 8) | buf[1];
  uint16_t temp_masked = read_data & MASK_TEMP;
  float temperature_c = ((175.72 * temp_masked) / 65536.0) - 46.85;

  uint8_t htm_temperature_buffer[5];
  //init the htm temperature buffer as 0
  memset(htm_temperature_buffer, 0, sizeof(htm_temperature_buffer));
  uint8_t *p = &htm_temperature_buffer[0];
  uint32_t htm_temperature_flt;
  //this means its Celsius  
  uint8_t flags = 0x00;
  UINT8_TO_BITSTREAM(p, flags); // insert the flags byte
  int32_t temperature_mC = (int32_t)(temperature_c * 1000.0f);
  htm_temperature_flt = INT32_TO_FLOAT(temperature_mC, -3);

  // insert the temperature measurement
  UINT32_TO_BITSTREAM(p, htm_temperature_flt);

  sl_status_t sc;

  // -------------------------------
  // Write our local GATT DB
  // -------------------------------
  sc = sl_bt_gatt_server_write_attribute_value(
      gattdb_temperature_measurement, // handle from gatt_db.h
      0,
      5,
      &htm_temperature_buffer[0] // in IEEE-11073 format
  );
  if (sc != SL_STATUS_OK)
  {
    LOG_ERROR("sl_bt_gatt_server_write_attribute_value failed with status code of status=0x%04x  \n \r", (unsigned int)sc);
  }

  ble_data_struct_t *ble = getBleDataPtr();
  //make sure indications are enabled, and active connection is in place and we dont have a previous packet in flight
  if (ble->connectionOpen &&
      ble->htmIndicationsEnabled &&
      !ble->is_Indication_Inflight)
  {
    sc = sl_bt_gatt_server_send_indication(
        ble->connectionHandle,
        gattdb_temperature_measurement, // handle from gatt_db.h
        5,
        &htm_temperature_buffer[0] // in IEEE-11073 format
    );
    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_gatt_server_send_indication failed with status code of status=0x%04x  \n \r", (unsigned int)sc);
    }
    else
    {
      ble->is_Indication_Inflight = true;
    }
  }
}

void read_data_from_Si7021()
{
  transferSequence.addr = SI7021_I2C_BUS_ADDRESS << 1; // shift device address left
  transferSequence.flags = I2C_FLAG_READ;              // call with read address
  transferSequence.buf[0].data = buf;                  // pointer to data to write
  transferSequence.buf[0].len = 2;
  NVIC_EnableIRQ(I2C0_IRQn);
  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
  transferStatus = I2C_TransferInit(I2C0, &transferSequence);
  // check status of transfer done, and log error
  if (transferStatus < 0)
  {
    LOG_ERROR("I2CSPM_Transfer: I2C bus read failed with status code of %d \n \r", transferStatus);
  }
  // why did things go wrong when process_temperature_reading was called here ?
}
