/*
 * ADXL343.c
 *
 *  Created on: Apr 19, 2026
 *      Author: abhir
 */


#include "ADXL343.h"
#define INCLUDE_LOG_DEBUG 1
#include "log.h"

uint8_t adxl_tx_buf[2] = {0};
uint8_t adxl_rx_buf[6] = {0};

/* Write a single register on the ADXL343 */
static void adxl_write_register(uint8_t reg, uint8_t value)
{
    adxl_tx_buf[0] = reg;
    adxl_tx_buf[1] = value;

    transferSequence.addr        = ADXL343_I2C_ADDR << 1;
    transferSequence.flags       = I2C_FLAG_WRITE;
    transferSequence.buf[0].data = adxl_tx_buf;
    transferSequence.buf[0].len  = 2;

    I2CSPM_Transfer(I2C0, &transferSequence);
}

void initialize_ADXL343(void)
{
    adxl_tx_buf[0] = ADXL343_REG_DEVID;

    transferSequence.addr        = ADXL343_I2C_ADDR << 1;
    transferSequence.flags       = I2C_FLAG_WRITE_READ;
    transferSequence.buf[0].data = adxl_tx_buf;
    transferSequence.buf[0].len  = 1;
    transferSequence.buf[1].data = adxl_rx_buf;
    transferSequence.buf[1].len  = 1;

    I2CSPM_Transfer(I2C0, &transferSequence);
    LOG_INFO("ADXL343 DEVID: 0x%02X (expected 0xE5)\n\r", adxl_rx_buf[0]);

    /* Set full resolution */
    adxl_write_register(ADXL343_REG_DATA_FORMAT, 0x0B);

    /* Enable measurement mode */
    adxl_write_register(ADXL343_REG_POWER_CTL, ADXL343_MEASURE_MODE);

    /* Freefall threshold: 437 mg */
    adxl_write_register(ADXL343_REG_THRESH_FF, 0x07);

    /* Freefall time: 160 ms */
    adxl_write_register(ADXL343_REG_TIME_FF,   0x20);

    /* Mapping freefall to INT1 pin*/
    adxl_write_register(ADXL343_REG_INT_MAP,   0x00);

    /* Enable freefall interrupt */
    adxl_write_register(ADXL343_REG_INT_ENABLE, INT_SRC_FREEFALL);

    /* Read INT_SOURCE once to clear any pending interrupt */
    adxl_read_register(ADXL343_REG_INT_SOURCE);

    LOG_INFO("ADXL343 init complete\n\r");
    I2C0->CMD = I2C_CMD_ABORT;
}

void process_ADXL343_values(void)
{
    /* Read INT_SOURCE and confirms freefall */
    uint8_t src = adxl_read_register(ADXL343_REG_INT_SOURCE);
    if (src & INT_SRC_FREEFALL) {
        LOG_INFO("*** FREEFALL DETECTED! ***\n\r");
    }
}

uint8_t adxl_read_register(uint8_t reg)
{
    uint8_t val = 0;
    adxl_tx_buf[0] = reg;
    transferSequence.addr        = ADXL343_I2C_ADDR << 1;
    transferSequence.flags       = I2C_FLAG_WRITE_READ;
    transferSequence.buf[0].data = adxl_tx_buf;
    transferSequence.buf[0].len  = 1;
    transferSequence.buf[1].data = &val;
    transferSequence.buf[1].len  = 1;
    I2CSPM_Transfer(I2C0, &transferSequence);
    return val;
}
