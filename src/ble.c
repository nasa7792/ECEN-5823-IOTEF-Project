/*
 * ble.c
 *
 *  Created on: Feb 15, 2026
 *      Author: Nalin Saxena
 *
 *  File Brief- Implementation file for All BLE related Apis, and helper macros
 */
#define INCLUDE_LOG_DEBUG 1
#include "log.h"
#include "ble.h"

ble_data_struct_t ble_data;

void handle_ble_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  bd_addr address;
  uint8_t address_type;
  const char server_str[] = "Server";
  const char Assignment_str[] = "A6";
  const char adv_msg[]="Advertising";
  const char connected_msg[]="Connected";

  // Handle stack events
  switch (SL_BT_MSG_ID(evt->header))
  {
  // -------------------------------
  // This event indicates the device has started and the radio is ready.
  // Do not call any stack command before receiving this boot event!
  case sl_bt_evt_system_boot_id:
    // Get device address
    sc = sl_bt_system_get_identity_address(&address, &address_type);

    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_system_get_identity_address() returned != 0 status=0x%04x  \n \r", (unsigned int)sc);
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

    displayInit();
    displayPrintf(DISPLAY_ROW_NAME,server_str);
    displayPrintf(DISPLAY_ROW_BTADDR,addrStr);
    displayPrintf(DISPLAY_ROW_ASSIGNMENT,Assignment_str);
    displayPrintf(DISPLAY_ROW_CONNECTION,adv_msg);
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

    break;

  case sl_bt_evt_connection_opened_id:
    // mark connection as open
    ble_data.connectionOpen = true;
    ble_data.connectionHandle = evt->data.evt_connection_opened.connection;

    // stop advertising, since we already have an active connection
    sc = sl_bt_advertiser_stop(ble_data.advertisingSetHandle);
    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_advertiser_stop() returned != 0 status=0x%04x  \n \r", (unsigned int)sc);
    }
    displayPrintf(DISPLAY_ROW_CONNECTION,connected_msg);
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
    displayPrintf(DISPLAY_ROW_CONNECTION,adv_msg);
    displayPrintf(DISPLAY_ROW_TEMPVALUE," ");
    ble_data.connectionOpen = false;
    ble_data.connectionHandle = 0;
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
    break;

  case sl_bt_evt_gatt_server_characteristic_status_id:
  {
    uint16_t characteristic = evt->data.evt_gatt_server_characteristic_status.characteristic;
    uint8_t sf = evt->data.evt_gatt_server_characteristic_status.status_flags;
    uint16_t ccf = evt->data.evt_gatt_server_characteristic_status.client_config_flags;
    // client requested a change ?
    if (sf == sl_bt_gatt_server_client_config)
    {
      // Dealing with temperature values ??
      if (characteristic == gattdb_temperature_measurement)
      {
        if (ccf == sl_bt_gatt_server_indication)
        {
          ble_data.htmIndicationsEnabled = true;
        }
        else
        {
          ble_data.htmIndicationsEnabled = false;
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
  // on timeout mark indication inflight as false
  case sl_bt_evt_gatt_server_indication_timeout_id:
    ble_data.is_Indication_Inflight = false;
    break;
  default:
    break;
  }
}
ble_data_struct_t *getBleDataPtr()
{
  return (&ble_data); // a handler for the ble ptr;
}
