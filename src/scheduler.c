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

void scheduler_setEvent_I2C_Transfer_Error()
{
  CORE_DECLARE_IRQ_STATE;
  // set event
  CORE_ENTER_CRITICAL(); // enter critical, turn off interrupts in NVIC
  sl_bt_external_signal(evtI2CTransferError);
  CORE_EXIT_CRITICAL(); // exit critical, re-enable interrupts in NVIC
}

void setEvent_PB0_Pressed()
{
  CORE_DECLARE_IRQ_STATE;
  // set event
  CORE_ENTER_CRITICAL(); // enter critical, turn off interrupts in NVIC
  sl_bt_external_signal(evtPB0Pressed);
  CORE_EXIT_CRITICAL(); // exit critical, re-enable interrupts in NVIC
}

void setEvent_PB0_Released()
{
  CORE_DECLARE_IRQ_STATE;
  // set event
  CORE_ENTER_CRITICAL(); // enter critical, turn off interrupts in NVIC
  sl_bt_external_signal(evtPB0Released);
  CORE_EXIT_CRITICAL(); // exit critical, re-enable interrupts in NVIC
}

void setEvent_PB1_Pressed()
{
  CORE_DECLARE_IRQ_STATE;
  // set event
  CORE_ENTER_CRITICAL(); // enter critical, turn off interrupts in NVIC
  sl_bt_external_signal(evtPB1Pressed);
  CORE_EXIT_CRITICAL(); // exit critical, re-enable interrupts in NVIC
}

void setEvent_PB1_Released()
{
  CORE_DECLARE_IRQ_STATE;
  // set event
  CORE_ENTER_CRITICAL(); // enter critical, turn off interrupts in NVIC
  sl_bt_external_signal(evtPB1Released);
  CORE_EXIT_CRITICAL(); // exit critical, re-enable interrupts in NVIC
} // if we are building code for the server

#if DEVICE_IS_BLE_SERVER

// our possible states for master
typedef enum uint32_t
{
  STATE0_IDLE,
  STATE1_WAKEUP_WAIT,
  STATE2_I2C_READ_COMPLETE,
} State_temp_t;

void server_state_machine(sl_bt_msg_t *evt)
{
  static State_temp_t nextState = STATE0_IDLE;
  State_temp_t currentState = nextState;

  if (SL_BT_MSG_ID(evt->header) != sl_bt_evt_system_external_signal_id)
    return;

  uint32_t signals = evt->data.evt_system_external_signal.extsignals;

  switch (currentState)
  {
  case STATE0_IDLE:
    if (signals & evtLETIMER0_UnderFlow)
    {

      if (getBleDataPtr()->connectionOpen == false || getBleDataPtr()->HRSO2IndicationsEnabled == false)
      {
        return;
      }

      GPIO_PinOutClear(HRSPO2_MFIO_PORT, HRSPO2_MFIO_PIN); // wake from deep sleep
      timerWaitUs_irq(10 * 1000);
      nextState = STATE1_WAKEUP_WAIT;
    }
    break;

  case STATE1_WAKEUP_WAIT:
    if (signals & evtLETIMER0_Comp1)
    {
      sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
      read_fifo_from_HRSPO2(); // combined write+read in one transaction
      nextState = STATE2_I2C_READ_COMPLETE;
    }
    break;

  case STATE2_I2C_READ_COMPLETE:
    if (signals & evtI2CTransferComplete)
    {
      sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
      if (getBleDataPtr()->connectionOpen && getBleDataPtr()->HRSO2IndicationsEnabled){
      process_HRSPO2_values();
      }
      GPIO_PinOutSet(HRSPO2_MFIO_PORT, HRSPO2_MFIO_PIN); // back to deep sleep
      nextState = STATE0_IDLE;
    }
    if (signals & evtI2CTransferError)
    {
      LOG_INFO("I2C error happened going back to idle lets try in next measurement \n \r");
      nextState = STATE0_IDLE;
    }
    break;

  default:
    break;
  }
}

// our possible states for client
#else
typedef enum uint32_t
{
  STATE0_IDLE,
  STATE1_DISCOVER_HTM_SERVICE,
  STATE2_DISCOVER_HTM_CHARACTERISTICS,
  STATE3_ENABLE_HTM_INDICATIONS,
  STATE4_DISCOVER_BTN_SERVICE,
  STATE5_DISCOVER_BTN_CHARACTERISTICS,
  STATE6_ENABLE_BTN_INDICATIONS
} State_disovery_t;

void discovery_state_machine(sl_bt_msg_t *evt)
{
  static State_disovery_t nextState = STATE0_IDLE;
  State_disovery_t currentState = nextState;
  sl_status_t sc;
  ble_data_struct_t *ble_data = getBleDataPtr();
  // services and characteristic uuids
  const uint8_t thermoService[2] = {0x09, 0x18}; // as per lecture slides
  const uint8_t thermo_char_uuid[2] = {0x1C, 0x2A};

  // From the auto generated gatt_db.c
  const uint8_t btn_service_uuid[16] = {
      0x89, 0x62, 0x13, 0x2d, 0x2a, 0x65, 0xec, 0x87,
      0x3e, 0x43, 0xc8, 0x38, 0x01, 0x00, 0x00, 0x00};

  const uint8_t btn_char_uuid[16] = {
      0x89, 0x62, 0x13, 0x2d, 0x2a, 0x65, 0xec, 0x87,
      0x3e, 0x43, 0xc8, 0x38, 0x02, 0x00, 0x00, 0x00};

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
      nextState = STATE1_DISCOVER_HTM_SERVICE; // if no error move to next state
    }
    break;
  case sl_bt_evt_gatt_procedure_completed_id: // this is a common event for multiple state changes
    if (currentState == STATE1_DISCOVER_HTM_SERVICE)
    {
      sc = sl_bt_gatt_discover_characteristics_by_uuid(
          ble_data->connectionHandle,
          ble_data->serviceHandle_htm,
          sizeof(thermo_char_uuid),
          thermo_char_uuid);
      if (sc != SL_STATUS_OK)
      {
        LOG_ERROR("sl_bt_gatt_discover_characteristics_by_uuid() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      else
      {
        nextState = STATE2_DISCOVER_HTM_CHARACTERISTICS;
      }
    }

    else if (currentState == STATE2_DISCOVER_HTM_CHARACTERISTICS)
    {
      sc = sl_bt_gatt_set_characteristic_notification(
          ble_data->connectionHandle,
          ble_data->characteristicHandle_htm,
          sl_bt_gatt_indication);
      if (sc != SL_STATUS_OK)
      {
        LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }
      else
      {
        nextState = STATE3_ENABLE_HTM_INDICATIONS;
      }
    }

    else if (currentState == STATE3_ENABLE_HTM_INDICATIONS)
    {
      ble_data->HRSO2IndicationsEnabled = true;
      displayPrintf(DISPLAY_ROW_CONNECTION, "Handling Indications");
      // now discover button service
      sc = sl_bt_gatt_discover_primary_services_by_uuid(
          ble_data->connectionHandle,
          sizeof(btn_service_uuid),
          btn_service_uuid);
      if (sc != SL_STATUS_OK)
      {
        LOG_ERROR("discover btn service failed=0x%04x\n\r", (unsigned int)sc);
      }
      else
      {
        nextState = STATE4_DISCOVER_BTN_SERVICE;
      }
    }

    else if (currentState == STATE4_DISCOVER_BTN_SERVICE)
    {
      sc = sl_bt_gatt_discover_characteristics_by_uuid(
          ble_data->connectionHandle,
          ble_data->serviceHandle_btn,
          sizeof(btn_char_uuid),
          btn_char_uuid);
      if (sc != SL_STATUS_OK)
      {
        LOG_ERROR("sl_bt_gatt_discover_characteristics_by_uuid() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      else
      {
        nextState = STATE5_DISCOVER_BTN_CHARACTERISTICS;
      }
    }

    else if (currentState == STATE5_DISCOVER_BTN_CHARACTERISTICS)
    {
      // auto enable button indications
      sc = sl_bt_gatt_set_characteristic_notification(
          ble_data->connectionHandle,
          ble_data->characteristicHandle_btn,
          sl_bt_gatt_indication);
      if (sc != SL_STATUS_OK)
      {
        LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }
      else
      {
        nextState = STATE6_ENABLE_BTN_INDICATIONS;
      }
    }

    else if (currentState == STATE6_ENABLE_BTN_INDICATIONS)
    {
      ble_data->btnIndicationsEnabled = true; // mark btn indications as true
      nextState = STATE0_IDLE;
    }
    break;

  case sl_bt_evt_connection_closed_id:
    nextState = STATE0_IDLE; // just wait and go to idle
    break;
  }
}
#endif
