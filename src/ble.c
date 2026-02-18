/*
 * ble.c
 *
 *  Created on: Feb 15, 2026
 *      Author: Asus
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

  // Handle stack events
  switch (SL_BT_MSG_ID(evt->header))
  {
  // -------------------------------
  // This event indicates the device has started and the radio is ready.
  // Do not call any stack command before receiving this boot event!
  case sl_bt_evt_system_boot_id:

    // Print stack version
    LOG_INFO("Bluetooth stack booted: v%d.%d.%d-b%d\n",
             evt->data.evt_system_boot.major,
             evt->data.evt_system_boot.minor,
             evt->data.evt_system_boot.patch,
             evt->data.evt_system_boot.build);

    // Get device address
    sc = sl_bt_system_get_identity_address(&address, &address_type);
    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_system_get_identity_address() returned != 0 status=0x%04x", (unsigned int)sc);
    }
    app_assert_status(sc);
    ble_data.myAddress = address;

    LOG_INFO("Bluetooth %s address: %02X:%02X:%02X:%02X:%02X:%02X\n",
             address_type ? "static random" : "public device",
             address.addr[5], address.addr[4], address.addr[3],
             address.addr[2], address.addr[1], address.addr[0]);

    // Create advertising set
    sc = sl_bt_advertiser_create_set(&ble_data.advertisingSetHandle);
    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_advertiser_create_set() returned != 0 status=0x%04x", (unsigned int)sc);
    }
    app_assert_status(sc);

    // Set advertising interval to 250ms (0.625ms units)
    sc = sl_bt_advertiser_set_timing(
        ble_data.advertisingSetHandle,
        400, // min interval = 250ms
        400, // max interval = 250ms
        0,   // duration (0 = forever)
        0);  // max events (0 = unlimited)
    app_assert_status(sc);
    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_advertiser_set_timing() returned != 0 status=0x%04x", (unsigned int)sc);
    }

    // Start advertising (connectable & scannable)
    sc = sl_bt_advertiser_start(
        ble_data.advertisingSetHandle,
        sl_bt_advertiser_general_discoverable,
        sl_bt_advertiser_connectable_scannable);

    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_advertiser_start() returned != 0 status=0x%04x", (unsigned int)sc);
    }

    app_assert_status(sc);

    LOG_INFO("Started advertising\n");
    break;

  case sl_bt_evt_connection_opened_id:
    LOG_INFO("A new connection was opened ! stopping advertising \n \r");

    ble_data.connectionOpen=true;
    ble_data.connectionHandle=evt->data.evt_connection_opened.connection;

    //stop advertising
    sc=sl_bt_advertiser_stop(ble_data.advertisingSetHandle);

    sc = sl_bt_connection_set_parameters(
             ble_data.connectionHandle,
             60,    // min interval = 75ms (1.25ms units)
             60,    // max interval = 75ms
             3,     // latency = 3 slave intervals
             75,    // timeout = 750ms (10ms units)
             0,     // min CE length
             0xffff); // max CE length
    app_assert_status(sc);

    break;

  break;
  default:
    break;
  }
}
