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
#include "math.h"

ble_data_struct_t ble_data;

/*
 * start of circular queue related declarations and functions
 */
queue_struct_t my_queue[QUEUE_DEPTH]; // the queue
uint32_t wptr = 0;                    // write pointer
uint32_t rptr = 0;                    // read pointer

/*
I have declared a global variable called slots_used which will help with maintaining number of slots used in our buffer.
initially we will have zero slots used
*/
uint32_t slots_used = 0;

static uint32_t nextPtr(uint32_t ptr)
{

  // we need to update ptr in a circular fashion hence % QUEUE_DEPTH to get the next address
  ptr = (ptr + 1) % QUEUE_DEPTH;
  return ptr;

} // nextPtr()

uint32_t get_queue_depth()
{

  // Student edit:
  // Create this function
  // just return the value of slots_used
  return slots_used;
} // get_queue_depth()

bool enqueue(uint16_t charHandle, uint32_t bufLength, uint8_t *buffer)
{

  // Student edit:
  // Create this function
  // Decide how you want to handle the "full" condition.

  // Don't forget to range check bufLength.
  // Isolation of functionality:
  //     Create the logic for "when" a pointer advances.
  // first check if the queue is full
  if (slots_used == QUEUE_DEPTH)
  {
    return false; // our queue was full do not advance next pointer
  }
  // validate the input length of the given buffer
  if (bufLength > MAX_BUFFER_LENGTH || bufLength < MIN_BUFFER_LENGTH)
  {
    return false; // invalid range of buff length.
  }

  // copy corresponding value to correct buffer position i.e location of the wrptr
  my_queue[wptr].charHandle = charHandle;
  my_queue[wptr].bufLength = bufLength;
  for (uint32_t i = 0; i < bufLength; i++)
  {
    my_queue[wptr].buffer[i] = buffer[i];
  }
  // update slots used
  slots_used++;
  // update write pointer to point to next location
  wptr = nextPtr(wptr);
  return true; // indidcate success!

} // write_queue()

bool dequeue(uint16_t *charHandle, uint32_t *bufLength, uint8_t *buffer)
{

  // Student edit:
  // Create this function

  // Isolation of functionality:
  //     Create the logic for "when" a pointer advances

  // first check if our queue is empty
  if (slots_used == 0)
  {
    return false; // our buffer was empty a read is not possible.
  }
  // copy value to passed arguments from the read position of the buffer
  *charHandle = my_queue[rptr].charHandle;
  *bufLength = my_queue[rptr].bufLength;
  memcpy(buffer, my_queue[rptr].buffer, *bufLength);
  // reduce number of slots used
  slots_used--;
  // update read pointer
  rptr = nextPtr(rptr);
  // indidcate success!
  return true;

} // read_queue()

/*
 * end of circular queue related declarations and functions
 */


/*
 * a helper function to deal with button indications
 *
 */
void send_Indication_OrStore(uint32_t bufLength, uint8_t *buffer)
{
  if (!ble_data.connectionOpen || !ble_data.btnIndicationsEnabled)
  {
    return;
  }
  sl_status_t sc;
  //if we do not have any on inflight indications and nothing in our button queue we just send the indications
  if (!ble_data.is_Indication_Inflight && get_queue_depth() == 0)
  {
    sc = sl_bt_gatt_server_send_indication(
        ble_data.connectionHandle,
        gattdb_button_state,
        1,
        buffer);

    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_gatt_server_send_indication failed with status code of status=0x%04x  \n \r", (unsigned int)sc);
    }
    else
    {
      //if no error happened that means indications are in flight
      ble_data.is_Indication_Inflight = true;
    }
  }
  else
  {
      //if we have an already inflight indication we store the current indication in queue
    enqueue(gattdb_button_state, bufLength, buffer);
  }
}

float temperature_conv(const uint8_t *buffer_ptr)
{
  uint8_t signByte = 0;
  int32_t mantissa;
  // input data format is:
  // [0] = flags byte, bit[0] = 0 -> Celsius; =1 -> Fahrenheit
  // [3][2][1] = mantissa (2's complement)
  // [4] = exponent (2's complement)
  // BT buffer_ptr[0] has the flags byte
  int8_t exponent = (int8_t)buffer_ptr[4];
  // sign extend the mantissa value if the mantissa is negative
  if (buffer_ptr[3] & 0x80)
  { // msb of [3] is the sign of the mantissa
    signByte = 0xFF;
  }
  mantissa = (int32_t)(buffer_ptr[1] << 0) |
             (int32_t)(buffer_ptr[2] << 8) |
             (int32_t)(buffer_ptr[3] << 16) |
             (int32_t)(signByte << 24);
  // value = 10^exponent * mantissa, pow() returns a double type
  return powf(10.0f, (float)exponent) * (float)mantissa;
} // FLOAT_TO_INT32

void handle_ble_event(sl_bt_msg_t *evt)
{
  // common variable and constants
  sl_status_t sc;
  bd_addr address;
  uint8_t address_type;
  const char Assignment_str[] = "A8"; // as per assignment A8
  const char adv_msg[] = "Advertising";
  const char scan_msg[] = "Discovering";
  const char connected_msg[] = "Connected";
  // server address
  uint8_t server_addr[6] = SERVER_BT_ADDRESS;
  char server_str[18];
  // format the server address
  snprintf(server_str, sizeof(server_str),
           "%02X:%02X:%02X:%02X:%02X:%02X",
           server_addr[5],
           server_addr[4],
           server_addr[3],
           server_addr[2],
           server_addr[1],
           server_addr[0]);

  // Handle stack events
  switch (SL_BT_MSG_ID(evt->header))
  {
  // -------------------------------
  // This event indicates the device has started and the radio is ready.
  // Do not call any stack command before receiving this boot event!
  case sl_bt_evt_system_boot_id:

    sl_bt_sm_delete_bondings(); // delete all previous bondings

    // Get device address, below code is common to server and client
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

    // add relevant calls to display, text server, assignment name, ble address
    displayInit();
/*server logic starts*/
#if DEVICE_IS_BLE_SERVER
    displayPrintf(DISPLAY_ROW_NAME, BLE_DEVICE_TYPE_STRING); // display Server
    displayPrintf(DISPLAY_ROW_BTADDR, addrStr);              // display address of  Server
    displayPrintf(DISPLAY_ROW_ASSIGNMENT, Assignment_str);   // display A8
    displayPrintf(DISPLAY_ROW_CONNECTION, adv_msg);          // display Advertising
    sl_bt_sm_configure(0x0F, sl_bt_sm_io_capability_displayyesno); //add security apis

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
    displayPrintf(DISPLAY_ROW_BTADDR, addrStr);
    displayPrintf(DISPLAY_ROW_ASSIGNMENT, Assignment_str);
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

    displayPrintf(DISPLAY_ROW_CONNECTION, connected_msg);

    // if we are server then we
    //  stop advertising, since we already have an active connection
#if DEVICE_IS_BLE_SERVER
    sc = sl_bt_advertiser_stop(ble_data.advertisingSetHandle);
    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_advertiser_stop() returned != 0 status=0x%04x  \n \r", (unsigned int)sc);
    }
#else
    // if we are client we stop scanning
    displayPrintf(DISPLAY_ROW_BTADDR2, server_str);
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
    ble_data.connectionOpen = false;
    ble_data.connectionHandle = 0;
    ble_data.htmIndicationsEnabled = false; // this was a bug :) in previous assignemnt
    ble_data.btnIndicationsEnabled = false;

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
      if (characteristic == gattdb_temperature_measurement)
      {
        if (ccf == sl_bt_gatt_server_indication)
        {
          ble_data.htmIndicationsEnabled = true;
          gpioLed0SetOn(); //led 0 is on if htm indications are enabled
        }
        else
        {
          ble_data.htmIndicationsEnabled = false;
          gpioLed0SetOff(); //led 0 is off if htm indications are disabled
        }
      }
      //add another set for the button state
      if (characteristic == gattdb_button_state)
      {
        if (ccf == sl_bt_gatt_server_indication)
        {
          ble_data.btnIndicationsEnabled = true;
        }
        else
        {
          ble_data.btnIndicationsEnabled = false;
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
    uint8_t btn_state;
    //deal with external button pressed events
    if (signals & evtBtnPressed)
    {
      displayPrintf(DISPLAY_ROW_10, "Button Pressed");
      btn_state = 0x01;
      gpioLed1SetOn(); //turn on user led 1
      if (ble_data.waitingForConfirmation == true)
      {
        sl_bt_sm_passkey_confirm(ble_data.connectionHandle, 1);
        ble_data.waitingForConfirmation = false;
      }
      sc = sl_bt_gatt_server_write_attribute_value(
          gattdb_button_state, 0, 1, &btn_state);
      if (sc != SL_STATUS_OK)
      {
        LOG_ERROR("gatt_server_write_attribute_value() failed=0x%04x\n\r", (unsigned int)sc);
      }
      // if indications are enabled we need to either send or enqueue them
      if (ble_data.btnIndicationsEnabled && ble_data.connectionOpen)
      {
        send_Indication_OrStore(1, &btn_state);
      }
    }
    if (signals & evtBtnReleased)
    {
      btn_state = 0x00;
      //turn off user led 1
      gpioLed1SetOff();
      displayPrintf(DISPLAY_ROW_10, "Button Released");

      // update local gatt server
      sc = sl_bt_gatt_server_write_attribute_value(
          gattdb_button_state, 0, 1, &btn_state);

      // if indications are enabled we need to either send or enqueue them
      if (ble_data.btnIndicationsEnabled && ble_data.connectionOpen)
      {
        send_Indication_OrStore(1, &btn_state);
      }
    }
    break;
  }

  case sl_bt_evt_sm_bonded_id:
    ble_data.bonded = true;
    displayPrintf(DISPLAY_ROW_CONNECTION, "Bonded");
    //clear up the display rows
    displayPrintf(DISPLAY_ROW_PASSKEY, " ");
    displayPrintf(DISPLAY_ROW_ACTION, " ");
    break;

  case sl_bt_evt_sm_bonding_failed_id:
    ble_data.bonded = false;
    LOG_ERROR("Bonding Failed! the reason is 0x%04x\n\r", (unsigned int)evt->data.evt_sm_bonding_failed.reason);
    break;
#endif

  // on timeout mark indication inflight as false
  case sl_bt_evt_gatt_server_indication_timeout_id:
    ble_data.is_Indication_Inflight = false;
    break;

    // lcd refresh is common to both client and server
  case sl_bt_evt_system_soft_timer_id: // generated on soft timer elapsing
    // on soft timer elapsing call lcd update function to prevent damage to lcd screen.
    displayUpdate();

    // as suggested in lecture slides we can take a look at our queue when soft timer elapses
#if DEVICE_IS_BLE_SERVER
    if (get_queue_depth() != 0 && !ble_data.is_Indication_Inflight)
    {
      queue_struct_t entry;
      bool dq_status=dequeue(&entry.charHandle,&entry.bufLength,entry.buffer);
      if(dq_status)
      {
        sc = sl_bt_gatt_server_send_indication(
            ble_data.connectionHandle,
            entry.charHandle,
            entry.bufLength,
            entry.buffer);

        if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_gatt_server_send_indication failed with status code of status=0x%04x  \n \r", (unsigned int)sc);
        }
        else
        {
          ble_data.is_Indication_Inflight = true;
        }
      }
    }
#endif
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

    // set service handle
  case sl_bt_evt_gatt_service_id:
    // store service handle
    ble_data.serviceHandle = evt->data.evt_gatt_service.service;
    break;

    // set characterstic handle
  case sl_bt_evt_gatt_characteristic_id:
    ble_data.characteristicHandle = evt->data.evt_gatt_characteristic.characteristic;
    break;

  case sl_bt_evt_gatt_characteristic_value_id:
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
    float temperature = temperature_conv(data);
    displayPrintf(DISPLAY_ROW_TEMPVALUE, "Temp=%.2f C", temperature);
    break;
#endif
  }
}
ble_data_struct_t *getBleDataPtr()
{
  return (&ble_data); // a handler for the ble ptr;
}
