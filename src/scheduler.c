/*
 * scheduler.c
 *
 *  Created on: Feb 4, 2026
 *      Author: Nalin Saxena
 *
 *  Edited on - 3/4/2026
 *
 *
 * File Brief -implementation file for scheduler related apis. Contains function defs related
 * to scheduler events
 *
 * Edited version contains, code for the client side and server side state machine
 */

#include "scheduler.h"
#include "timer.h"
#define INCLUDE_LOG_DEBUG 1
#include "log.h"

void scheduler_setEvent_UnderFlow()
{
  CORE_DECLARE_IRQ_STATE;
  // set event
  CORE_ENTER_CRITICAL(); // enter critical, turn off interrupts in NVIC
  sl_bt_external_signal(evtLETIMER0_UnderFlow);
  CORE_EXIT_CRITICAL(); // exit critical, re-enable interrupts in NVIC
}

void scheduler_setEvent_COMP1()
{
  CORE_DECLARE_IRQ_STATE;
  // set event
  CORE_ENTER_CRITICAL(); // enter critical, turn off interrupts in NVIC
  sl_bt_external_signal(evtLETIMER0_Comp1);
  CORE_EXIT_CRITICAL(); // exit critical, re-enable interrupts in NVIC
}

void scheduler_setEvent_I2C_Transfer_Complete()
{
  CORE_DECLARE_IRQ_STATE;
  // set event
  CORE_ENTER_CRITICAL(); // enter critical, turn off interrupts in NVIC
  sl_bt_external_signal(evtI2CTransferComplete);
  CORE_EXIT_CRITICAL(); // exit critical, re-enable interrupts in NVIC
}

// if we are building code for the server

#if DEVICE_IS_BLE_SERVER

// our possible states for master
typedef enum uint32_t
{
  STATE0_WARMUP,
  STATE1_I2C_WRITE,
  STATE2_INITIATE_CONVERSION,
  STATE3_I2C_READ_TEMP,
  STATE4_POWER_DOWN
} State_temp_t;

void temperature_state_machine(sl_bt_msg_t *evt)
{
  static State_temp_t nextState = STATE0_WARMUP;
  State_temp_t currentState = nextState;

  // we only react to external signals now getnextevent is dead
  if (SL_BT_MSG_ID(evt->header) != sl_bt_evt_system_external_signal_id)
    return;

  uint32_t signals = evt->data.evt_system_external_signal.extsignals;

  switch (currentState)
  {
  case STATE0_WARMUP:
    // at each uf event we enable sensor and arm the comp1 event to fire at 80ms
    if (signals & evtLETIMER0_UnderFlow)
    {
      // if no connection open then just return
      if (getBleDataPtr()->connectionOpen == false || getBleDataPtr()->htmIndicationsEnabled == false)
      {
        return;
      }

      enable_Si7021();
      timerWaitUs_irq(LOAD_PWR_MGMT_SENSOR);
      nextState = STATE1_I2C_WRITE;
    }
    break;

  case STATE1_I2C_WRITE:
    // comp1 event recieved send write command to sensor
    if (signals & evtLETIMER0_Comp1)
    {
      sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
      send_command_to_Si7021();
      nextState = STATE2_INITIATE_CONVERSION;
    }
    break;
  case STATE2_INITIATE_CONVERSION:
    // i2c complete event recieved disable nvic i2c, and arm timer for conversion
    if (signals & evtI2CTransferComplete)
    {
      sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
      NVIC_DisableIRQ(I2C0_IRQn);
      timerWaitUs_irq(CONV_TIME);
      nextState = STATE3_I2C_READ_TEMP;
    }
    break;
  case STATE3_I2C_READ_TEMP:
    // conversion completed, move to power down state
    if (signals & evtLETIMER0_Comp1)
    {
      read_data_from_Si7021();
      nextState = STATE4_POWER_DOWN;
    }
    break;
  case STATE4_POWER_DOWN:
    // log values on terminal, disable sensor,go back to warm up state
    if (signals & evtI2CTransferComplete)
    {
      sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
      // disable_Si7021();
      process_temperature_reading(); //
      NVIC_DisableIRQ(I2C0_IRQn);
      nextState = STATE0_WARMUP;
    }
    break;
  default:
    break;
  } // switch
} // state_machine()

// our possible states for client
#else
typedef enum uint32_t
{
  STATE0_IDLE,
  STATE1_DISCOVER_SERVICES,
  STATE2_DISCOVER_CHARACTERISTICS,
  STATE3_ENABLE_INDICATIONS,
} State_disovery_t;

void discovery_state_machine(sl_bt_msg_t *evt)
{
  static State_disovery_t nextState = STATE0_IDLE;
  State_disovery_t currentState = nextState;
  sl_status_t sc;
  ble_data_struct_t *ble_data = getBleDataPtr();
  const uint8_t thermoService[2] = {0x09, 0x18}; // as per lecture slides
  const uint8_t thermo_char_uuid[2] = {0x1C, 0x2A};

  switch (SL_BT_MSG_ID(evt->header))
  {
  case sl_bt_evt_connection_opened_id:
    // we need to scan for our temperature service by uuid
    sc = sl_bt_gatt_discover_primary_services_by_uuid(
        ble_data->connectionHandle, // already set by the handle_ble_event
        sizeof(thermoService),
        thermoService);
    if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_gatt_discover_primary_services_by_uuid() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
    }
    else
    {
      nextState = STATE1_DISCOVER_SERVICES; // if no error move to next state
    }
    break;
  case sl_bt_evt_gatt_procedure_completed_id: // this is a common event for multiple state changes
    if (currentState == STATE1_DISCOVER_SERVICES)
    {
      sc = sl_bt_gatt_discover_characteristics_by_uuid(
          ble_data->connectionHandle,
          ble_data->serviceHandle,
          sizeof(thermo_char_uuid),
          thermo_char_uuid);
      if (sc != SL_STATUS_OK)
      {
        LOG_ERROR("sl_bt_gatt_discover_characteristics_by_uuid() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      else
      {
        nextState = STATE2_DISCOVER_CHARACTERISTICS;
      }
    }

    else if (currentState == STATE2_DISCOVER_CHARACTERISTICS)
    {
      sc = sl_bt_gatt_set_characteristic_notification(
          ble_data->connectionHandle,
          ble_data->characteristicHandle,
          sl_bt_gatt_indication);
      if (sc != SL_STATUS_OK)
      {
        LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }
      else
      {
        nextState = STATE3_ENABLE_INDICATIONS;
      }
    }

    else if (currentState == STATE3_ENABLE_INDICATIONS)
    {
      ble_data->htmIndicationsEnabled = true; // we now enable indications
      displayPrintf(DISPLAY_ROW_CONNECTION, "Handling Indications");
      nextState = STATE0_IDLE; // just wait and go to idle
    }
    break;

  case sl_bt_evt_connection_closed_id:
    nextState = STATE0_IDLE; // just wait and go to idle
    break;
  }
}
#endif
