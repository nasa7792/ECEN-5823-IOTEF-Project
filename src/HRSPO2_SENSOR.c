/*
 * HRSPO2.c
 *
 *  Created on: 15-Apr-2026
 *      Author: nalin
 */

#include "HRSPO2_SENSOR.h"
#define INCLUDE_LOG_DEBUG 1
#include "log.h"

void initiliaze_HRSPO2_SENSOR(void)
{

    /*
    MAX32664 Application Mode -> https://www.analog.com/media/en/technical-documentation/user-guides/max32664-user-guide.pdf
    The MAX32664 enters application mode based on the sequencing of the RSTN pin and the MFIO
    pin. The necessary sequence is as follows:
    • Set the RSTN pin low for 10ms.
    • While RSTN is low, set the MFIO pin to high.
    • After the 10ms has elapsed, set the RSTN pin high. (MFIO pin should be set high at least
    1ms before RSTN pin is set high.)
    • After an additional 50ms has elapsed, the MAX32664 is in application mode and the
    application performs its initialization of the application software.
    • Approximately 1.5 second after the RSTN is set to high, the application completes the
    initialization and the device is ready to accept I2C commands. (For MAX32664A and
    MAX32664D, the startup time is 1.0 second).
    */

    // first set mfio high
    GPIO_PinOutSet(HRSPO2_MFIO_PORT, HRSPO2_MFIO_PIN);

    GPIO_PinOutClear(HRSPO2_RESET_PORT, HRSPO2_RESET_PIN);
    timerWaitUs_polled(HRSPO2_RESET_HOLD_MS * 1000);

    GPIO_PinOutSet(HRSPO2_RESET_PORT, HRSPO2_RESET_PIN);
    // 50 ms
    timerWaitUs_polled(HRSPO2_BOOT_SETTLE_MS * 1000);

    // 1.5 seconds??
    timerWaitUs_polled(HRSPO2_INIT_TIME * 1000);

    write_config_register_HRSPO2(FAM_SET_OUTPUT_MODE, IDX_SET_OUTPUT_MODE, OUTPUT_ALGO_DATA);
    timerWaitUs_polled(CMD_DELAY * 1000);

    // 2. Enable the sensor (MAX30101)
    write_config_register_HRSPO2(FAM_ENABLE_SENSOR, IDX_ENABLE_SENSOR, SENSOR_ENABLE);
    timerWaitUs_polled(CMD_DELAY * 1000);

    // 3. Enable the SpO2/HR Algorithm
    write_config_register_HRSPO2(FAM_ENABLE_ALGO, IDX_ENABLE_ALGO, ALGO_ENABLE);
    timerWaitUs_polled(CMD_DELAY * 1000);
}

void read_fifo_from_HRSPO2(void)
{
    HRSPO2_cmd_buf[0] = 0x12; // Family: Read Output FIFO
    HRSPO2_cmd_buf[1] = 0x01; // Index: Read Sample

    transferSequence.addr = HRSPO2_I2C_BUS_ADDRESS << 1;
    transferSequence.flags = I2C_FLAG_WRITE_READ; // combined write then read
    transferSequence.buf[0].data = HRSPO2_cmd_buf;
    transferSequence.buf[0].len = 2;
    transferSequence.buf[1].data = HRSPO2_rx_buf;
    transferSequence.buf[1].len = 10;
    sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
    NVIC_EnableIRQ(I2C0_IRQn);
    I2C_TransferInit(I2C0, &transferSequence);
}

void process_HRSPO2_values(void)
{
    uint8_t hub_status = HRSPO2_rx_buf[0];
    if (hub_status == 0x00)
    {
        uint16_t spo2_raw = (HRSPO2_rx_buf[1] << 8) | HRSPO2_rx_buf[2];
        uint8_t hr_conf = HRSPO2_rx_buf[3];
        uint16_t hr_raw = (HRSPO2_rx_buf[4] << 8) | HRSPO2_rx_buf[5];
        int8_t algo_stat = (int8_t)HRSPO2_rx_buf[7]; // signed! this is WHRM status

        if (algo_stat == 0)
        {
            char hr_disp[20];
            char spo2_disp[20];

            memset(hr_disp, 0, sizeof(hr_disp));
            memset(spo2_disp, 0, sizeof(spo2_disp));

            snprintf(hr_disp, sizeof(hr_disp), "HR: %d.%d bpm",
                     hr_raw / 10, hr_raw % 10);

            // SpO2: value %
            snprintf(spo2_disp, sizeof(spo2_disp), "SpO2: %d.%d%%",
                     spo2_raw / 10, spo2_raw % 10);

            displayPrintf(DISPLAY_ROW_9,spo2_disp);
            displayPrintf(DISPLAY_ROW_10,hr_disp);

            // send indications to client
            uint8_t hrspo2_buffer[4];
            hrspo2_buffer[0] = (hr_raw >> 8) & 0xFF;
            hrspo2_buffer[1] = hr_raw & 0xFF;
            hrspo2_buffer[2] = (spo2_raw >> 8) & 0xFF;
            hrspo2_buffer[3] = spo2_raw & 0xFF;

            // -------------------------------
            // Write our local GATT DB
            // -------------------------------
            sl_status_t sc = sl_bt_gatt_server_write_attribute_value(
                gattdb_Heart_Rate_Spo2,
                0,
                sizeof(hrspo2_buffer),
                hrspo2_buffer);
            if (sc != SL_STATUS_OK)
            {
                LOG_ERROR("write_attribute_value failed: 0x%04x\n\r", (unsigned int)sc);
            }


            ble_data_struct_t *ble = getBleDataPtr();
            if (ble->connectionOpen &&
                ble->HRSO2IndicationsEnabled &&
                !ble->is_Indication_Inflight)
            {
                sc = sl_bt_gatt_server_send_indication(
                    ble->connectionHandle,
                    gattdb_Heart_Rate_Spo2, // handle from gatt_db.h
                    sizeof(hrspo2_buffer),
                    hrspo2_buffer);
                if (sc != SL_STATUS_OK)
                {
                    LOG_ERROR("sl_bt_gatt_server_send_indication failed with status code of status=0x%04x  \n \r", (unsigned int)sc);
                }
                else
                {
                    LOG_INFO("indications sent \n \r");
                    ble->is_Indication_Inflight = true;
                }
            }
        }
        else
        {
            LOG_INFO("Algorithm Warming Up... (WHRM Status %d)\n\r", algo_stat);
        }
    }
}
