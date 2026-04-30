/*
 * ble.c
 *
 *  Created on: Feb 15, 2026
 *      Author:  Nalin Saxena and Abhirath Koushik
 *
 *  File Brief- Implementation file for All BLE related Apis, and helper macros
 */

#include <stdio.h>
#include <string.h>

#define INCLUDE_LOG_DEBUG 1
#include "log.h"
#include "ble.h"
#include "math.h"
#include "gpio.h"


ble_data_struct_t ble_data;
sl_sleeptimer_timer_handle_t blink_timer;

void blink_timer_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
    (void)handle;
    (void)data;

    // Toggle the LED every time the timer fires
    if (ble_data.fall_alert_active) {
        gpioLed1Toggle();
    }
}

void handle_ble_event(sl_bt_msg_t *evt)
{
  // common variable and constants
  sl_status_t sc;
  bd_addr address;
  uint8_t address_type;
  const char adv_msg[] = "Search Caregiver";
  const char scan_msg[] = "Searching Patient";

  // server address
  uint8_t server_addr[6] = SERVER_BT_ADDRESS;

  // Handle stack events
  switch (SL_BT_MSG_ID(evt->header))
  {
  // -------------------------------
  // This event indicates the device has started and the radio is ready.
  // Do not call any stack command before receiving this boot event!
  case sl_bt_evt_system_boot_id:

    sl_bt_sm_delete_bondings();                                    // delete all previous bondings
    sl_bt_sm_configure(0x0F, sl_bt_sm_io_capability_displayyesno); // add security apis

    // Get device address, below code is common to server and client
    sc = sl_bt_system_get_identity_address(&address, &address_type);

    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_system_get_identity_addressl_bt_sm_io_capability_displayyesnos() returned != 0 status=0x%04x  \n \r", (unsigned int)sc);
    }
    app_assert_status(sc);
    ble_data.myAddress = address;
    char addrStr[18];
    snprintf(addrStr, sizeof(addrStr),
             "%02X:%02X:%02X:%02X:%02X:%02X",
             address.addr[5],
             address.addr[4],
             address.addr[3],
             address.addr[2],
             address.addr[1],
             address.addr[0]);

    // add relevant calls to display, text server, assignment name, ble address
    displayInit();
/*server logic starts*/
#if DEVICE_IS_BLE_SERVER
    const char server_str[] = "Patient Node";       // as per assignment A8
    displayPrintf(DISPLAY_ROW_NAME, server_str);    // display A8
    displayPrintf(DISPLAY_ROW_CONNECTION, adv_msg); // display Advertising

    // Create advertising set
    sc = sl_bt_advertiser_create_set(&ble_data.advertisingSetHandle);
    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_advertiser_create_set() returned != 0 status=0x%04x  \n \r", (unsigned int)sc);
    }
    app_assert_status(sc);

    // Set advertising interval to 250ms (0.625ms units)
    sc = sl_bt_advertiser_set_timing(
        ble_data.advertisingSetHandle,
        400, // min interval = 250ms
        400, // max interval = 250ms
        0,
        0);
    app_assert_status(sc);
    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_advertiser_set_timing() returned != 0 status=0x%04x  \n \r", (unsigned int)sc);
    }

    // Start advertising (connectable & scannable)
    sc = sl_bt_advertiser_start(
        ble_data.advertisingSetHandle,
        sl_bt_advertiser_general_discoverable,
        sl_bt_advertiser_connectable_scannable);

    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_advertiser_start() returned != 0 status=0x%04x  \n \r", (unsigned int)sc);
    }

    app_assert_status(sc);
/*server logic ends*/

/*Client logic starts*/
#else
    displayPrintf(DISPLAY_ROW_NAME, BLE_DEVICE_TYPE_STRING);
    displayPrintf(DISPLAY_ROW_CONNECTION, scan_msg);

    // set scanning mode use passive scanning as per assignment
    sc = sl_bt_scanner_set_mode(
        sl_bt_gap_1m_phy,
        sl_bt_scanner_scan_mode_passive);
    app_assert_status(sc);

    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_scanner_set_mode() returned != 0 status=0x%04x  \n \r", (unsigned int)sc);
    }

    // set scanning timing parameters
    sc = sl_bt_scanner_set_timing(
        sl_bt_gap_1m_phy,
        80,
        40);
    app_assert_status(sc);
    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_scanner_set_timing() returned != 0 status=0x%04x  \n \r", (unsigned int)sc);
    }

    // set scanning paramaters for master
    sc = sl_bt_connection_set_default_parameters(
        60, // 75ms
        60, // 75ms
        4,  // slave latency
        82, // supervision timeout (825ms)
        0,  // min CE length
        4   // max CE length
    );

    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_connection_set_default_parameters() returned != 0 status=0x%04x  \n \r", (unsigned int)sc);
    }

    // start scanning
    sc = sl_bt_scanner_start(
        sl_bt_gap_1m_phy,
        sl_bt_scanner_discover_generic);
    app_assert_status(sc);
    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_scanner_start() returned != 0 status=0x%04x  \n \r", (unsigned int)sc);
    }
#endif
    /*Client logic ends*/
    break;

  case sl_bt_evt_connection_opened_id:

    // mark connection as open common to both server and client
    ble_data.connectionOpen = true;
    ble_data.connectionHandle = evt->data.evt_connection_opened.connection;
    // add relevant calls to display, connected message

    // if we are server then we
    //  stop advertising, since we already have an active connection
#if DEVICE_IS_BLE_SERVER
    const char connected_patient[] = "Connected 2 Caregiver";
    displayPrintf(DISPLAY_ROW_CONNECTION, connected_patient);
    sc = sl_bt_advertiser_stop(ble_data.advertisingSetHandle);
    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_advertiser_stop() returned != 0 status=0x%04x  \n \r", (unsigned int)sc);
    }
#else
    // if we are client we stop scanning
    const char connected_caregiver[] = "Connected 2 Patient";
    displayPrintf(DISPLAY_ROW_CONNECTION, connected_caregiver);
    sc = sl_bt_scanner_stop();
    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_scanner_stop() returned != 0 status=0x%04x  \n \r", (unsigned int)sc);
    }
    app_assert_status(sc);
#endif

    sc = sl_bt_connection_set_parameters(
        ble_data.connectionHandle,
        60,      // min interval = 75ms (1.25ms units)
        60,      // max interval = 75ms
        4,       // latency = 4 slave intervals
        80,      // timeout = 800ms (10ms units)
        0,       // min CE length
        0xffff); // max CE length
    app_assert_status(sc);
    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_connection_set_parameters() returned != 0 status=0x%04x  \n \r", (unsigned int)sc);
    }
    break;

  // as instructed by the assignment this is an informational event, and logs have been commented out
  case sl_bt_evt_connection_parameters_id:
  {
    uint32_t interval_ms = (evt->data.evt_connection_parameters.interval * 5) / 4;
    uint32_t timeout_ms = evt->data.evt_connection_parameters.timeout * 10;

    (void)interval_ms;
    (void)timeout_ms;

    //    LOG_INFO("Interval: %lu ms Latency: %lu Timeout: %lu ms",
    //             interval_ms,
    //             evt->data.evt_connection_parameters.latency,
    //             timeout_ms);
  }
  break;

  case sl_bt_evt_connection_closed_id:
    // on receiving close event mark connection open as false
    // add relevant calls to display, temperature value

    displayPrintf(DISPLAY_ROW_TEMPVALUE, " ");
    sl_bt_sm_delete_bondings(); // regardless if its server or client delete all old bondings, was causing issues on reset
    // do a proper cleanup
    ble_data.connectionOpen = false;
    ble_data.connectionHandle = 0;
    ble_data.HRSO2IndicationsEnabled = false; // this was a bug :) in previous assignemnt
    ble_data.wasPB1Pressed = false;
    ble_data.isPB0Held = false;
    ble_data.is_Indication_Inflight = false;
    ble_data.isReadRequestInflight = false;
    ble_data.waitingForConfirmation = false;
    gpioLed1SetOff();

#if DEVICE_IS_BLE_SERVER
    displayPrintf(DISPLAY_ROW_CONNECTION, adv_msg);
    // restart advertising !
    sc = sl_bt_advertiser_start(
        ble_data.advertisingSetHandle,
        sl_bt_advertiser_general_discoverable,
        sl_bt_advertiser_connectable_scannable);
    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_advertiser_start() returned != 0 status=0x%04x  \n \r", (unsigned int)sc);
    }
    app_assert_status(sc);
#else
    displayPrintf(DISPLAY_ROW_CONNECTION, scan_msg);
    displayPrintf(DISPLAY_ROW_9, " "); // since connection is closed we can clear this row
    // restart scanning
    sc = sl_bt_scanner_start(
        sl_bt_gap_1m_phy,
        sl_bt_scanner_discover_generic);
    app_assert_status(sc);
    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_scanner_start() returned != 0 status=0x%04x  \n \r", (unsigned int)sc);
    }
#endif
    break;

#if DEVICE_IS_BLE_SERVER
  case sl_bt_evt_gatt_server_characteristic_status_id:
  {
    uint16_t characteristic = evt->data.evt_gatt_server_characteristic_status.characteristic;
    uint8_t sf = evt->data.evt_gatt_server_characteristic_status.status_flags;
    uint16_t ccf = evt->data.evt_gatt_server_characteristic_status.client_config_flags;
    // client requested a change ?

    if (sf == sl_bt_gatt_server_client_config)
    {
      // Dealing with temperature values ??
      if (characteristic == gattdb_Heart_Rate_Spo2)
      {
        LOG_INFO("turning on indications for hrspo2 1 \n \r");
        if (ccf == sl_bt_gatt_server_indication)
        {
          ble_data.HRSO2IndicationsEnabled = true;
          displayPrintf(DISPLAY_ROW_ACTION, "Health Stats are:");
          gpioLed0SetOn(); // led 0 is on if htm indications are enabled
        }
        else
        {
          ble_data.HRSO2IndicationsEnabled = false;
          gpioLed0SetOff(); // led 0 is off if htm indications are disabled
        }
      }

      if (characteristic == gattdb_Fall_characteristic)
      {
        LOG_INFO("turning on indications for fall service 1 \n \r");
        if (ccf == sl_bt_gatt_server_indication)
        {
          ble_data.FallDetection_Indications_Enabled = true;
        }
      }
    }
    // once confirmation is recieved mark, inflight as false
    if (sf == sl_bt_gatt_server_confirmation)
    {
      ble_data.is_Indication_Inflight = false;
    }
  }
  break;
#endif

    // first bonding event we auto accept
  case sl_bt_evt_sm_confirm_bonding_id:
    sc = sl_bt_sm_bonding_confirm(ble_data.connectionHandle, 1);
    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_sm_bonding_confirm() returned != 0 status=0x%04x  \n \r", (unsigned int)sc);
    }
    app_assert_status(sc);
    break;

  case sl_bt_evt_sm_confirm_passkey_id:
  {
    uint32_t passkey = evt->data.evt_sm_confirm_passkey.passkey;
    ble_data.waitingForConfirmation = true;
    char passkey_str[12];
    snprintf(passkey_str, sizeof(passkey_str), "%lu", (unsigned long)passkey);
    displayPrintf(DISPLAY_ROW_PASSKEY, passkey_str);
    displayPrintf(DISPLAY_ROW_ACTION, "Confirm with PB0");
  }
  break;

  case sl_bt_evt_system_external_signal_id:
  {
    uint32_t signals = evt->data.evt_system_external_signal.extsignals;
    // deal with external button pressed events
#if DEVICE_IS_BLE_SERVER
    if (signals & evtPB0Pressed)
    {
      if (ble_data.waitingForConfirmation == true)
      {
        sl_bt_sm_passkey_confirm(ble_data.connectionHandle, 1);
        ble_data.waitingForConfirmation = false;
      }
    }
#else
    if (signals & evtPB0Pressed)
    {
      ble_data.isPB0Held = true;

      if (ble_data.fall_alert_active) {
          LOG_INFO("Caregiver acknowledged fall alert via PB0\n\r");
          ble_data.fall_alert_active = false;

          // Stop Timer 1
          sl_sleeptimer_stop_timer(&blink_timer);

          // Turn LED1 Off
          gpioLed1SetOff();

          // Clear the LCD warning
          displayPrintf(DISPLAY_ROW_ACTION, "Alert Cleared");
      }

      if (ble_data.waitingForConfirmation == true)
      {
        sl_bt_sm_passkey_confirm(ble_data.connectionHandle, 1);
        ble_data.waitingForConfirmation = false;
      }
    }

#endif

    break;
  }

  case sl_bt_evt_sm_bonded_id:
    LOG_INFO("bonding done \n \r");
    ble_data.bonded = true;
    // clear up the display rows
    displayPrintf(DISPLAY_ROW_PASSKEY, " ");
    displayPrintf(DISPLAY_ROW_ACTION, " ");
    break;

  case sl_bt_evt_sm_bonding_failed_id:
    ble_data.bonded = false;
    LOG_ERROR("Bonding Failed! the reason is 0x%04x\n\r", (unsigned int)evt->data.evt_sm_bonding_failed.reason);
    break;

  // on timeout mark indication inflight as false
  case sl_bt_evt_gatt_server_indication_timeout_id:
    ble_data.is_Indication_Inflight = false;
    break;

    // lcd refresh is common to both client and server
  case sl_bt_evt_system_soft_timer_id: // generated on soft timer elapsing
    // on soft timer elapsing call lcd update function to prevent damage to lcd screen.
    displayUpdate();
    break;

// client only events
#if !DEVICE_IS_BLE_SERVER
  case sl_bt_evt_scanner_scan_report_id:
  {
    sl_bt_evt_scanner_scan_report_t *report = &evt->data.evt_scanner_scan_report;

    // must be an adversitement packet
    if (report->packet_type != 0)
    {
      break;
    }
    // check if servers address is present
    if (memcmp(report->address.addr, server_addr, 6) != 0)
    {

      break;
    }

    sc = sl_bt_scanner_stop();
    app_assert_status(sc);
    // mark connection handle as 1
    //?? ble_data.connectionHandle = 1;

    sc = sl_bt_connection_open(
        report->address,      // bd_addr of the server
        report->address_type, // address type (public = 0)
        sl_bt_gap_1m_phy,     // PHY: 1M (use sl_bt_gap_phy_coded for long range)
        NULL                  // out: connection handle (filled on open event)
    );
    app_assert_status(sc);
  }
  break;

  case sl_bt_evt_gatt_characteristic_value_id:
  {
    uint16_t char_handle = evt->data.evt_gatt_characteristic_value.characteristic;
    if (evt->data.evt_gatt_characteristic_value.att_opcode == sl_bt_gatt_handle_value_indication)
    {
      // client sends ack for indications
      sc = sl_bt_gatt_send_characteristic_confirmation(ble_data.connectionHandle);
      app_assert_status(sc);
      if (sc != SL_STATUS_OK)
      {
        LOG_ERROR("sl_bt_gatt_send_characteristic_confirmation() returned != 0 status=0x%04x  \n \r", (unsigned int)sc);
      }
    }
    uint8_t *data = evt->data.evt_gatt_characteristic_value.value.data;
    if (char_handle == ble_data.characteristicHandle_hrspo2)
    {
      uint16_t hr_raw = (data[0] << 8) | data[1];
      uint16_t spo2_raw = (data[2] << 8) | data[3];

      char hr_str[16];
      char spo2_str[16];

      uint16_t hr_whole   = hr_raw / 10;
      uint16_t hr_decimal = hr_raw % 10;

      uint16_t spo2_whole   = spo2_raw / 10;
      uint16_t spo2_decimal = spo2_raw % 10;


      snprintf(hr_str, sizeof(hr_str), "HR=%u.%u bpm", hr_whole, hr_decimal);
      snprintf(spo2_str, sizeof(spo2_str), "SpO2=%u.%u%%", spo2_whole, spo2_decimal);

      displayPrintf(DISPLAY_ROW_8, hr_str);
      displayPrintf(DISPLAY_ROW_9, spo2_str);
    }
    else if (char_handle == ble_data.characteristicHandle_fall)
    {
        if (data[0] == 0x01)
        {
            LOG_INFO("Fall indication received!\n\r");
            displayPrintf(DISPLAY_ROW_ACTION, "FALL DETECTED");
            ble_data.fall_alert_active = true;

            // Start Timer 1 every 250ms
            sl_sleeptimer_start_periodic_timer_ms(&blink_timer, 250, blink_timer_callback, NULL, 0, 0);
        }
    }

  }
  break;

  case sl_bt_evt_gatt_procedure_completed_id:
  {
    sl_status_t result = evt->data.evt_gatt_procedure_completed.result;
    if (result == SL_STATUS_BT_ATT_INSUFFICIENT_ENCRYPTION)
    {
      LOG_ERROR("Initiate bonding process \n\r");
      sl_bt_sm_increase_security(ble_data.connectionHandle);
      ble_data.isReadRequestInflight = false;
    }
  }
  break;
#endif
  }
}
ble_data_struct_t *getBleDataPtr()
{
  return (&ble_data); // a handler for the ble ptr;
}
