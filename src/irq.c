/*
 * irq.c
 *
 *  Created on: Jan 25, 2026
 *      Author: Nalin Saxena
 * File Brief- Implementation file for IRQ configuration apis and irq handler. Contains function def related
 * to IRQ enabling for LETIMER0 and irq hanlder which calls for a temp reading
 */

#include "irq.h"
#define INCLUDE_LOG_DEBUG 1
#include "log.h"
volatile uint32_t elapsed_ms = 0;


void GPIO_EVEN_IRQHandler(void)
{
    // Get and clear the interrupt flags
    uint32_t flags = GPIO_IntGet();
    GPIO_IntClear(flags);

    // Check if PB0 (pin 6) triggered the interrupt
    if (flags & (1 << USR_BTN0)) {
        if (GPIO_PinInGet(gpioPortF, USR_BTN0) == 0) {
            scheduler_setEvent_BtnPressed();
        } else {
            scheduler_setEvent_BtnReleased();
        }
    }
}

void enable_LETIMER0_interrupt()
{
  // Enable LETIMER interrupt in NVIC
  NVIC_EnableIRQ(LETIMER0_IRQn);
}

void LETIMER0_IRQHandler(void)
{
  uint32_t flags = LETIMER_IntGet(LETIMER0);
  uint32_t enabled = LETIMER_IntGetEnabled(LETIMER0); //

  if ((flags & LETIMER_IF_UF) && (enabled & LETIMER_IF_UF))
  {
    LETIMER_IntClear(LETIMER0, LETIMER_IF_UF);
    elapsed_ms++; // helps count up the timestamp value
    scheduler_setEvent_UnderFlow();
  }

  if ((flags & LETIMER_IF_COMP1) && (enabled & LETIMER_IF_COMP1))
  {
    LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);
    LETIMER_IntClear(LETIMER0, LETIMER_IF_COMP1);
    scheduler_setEvent_COMP1();
  }
}

// i2c event handler
void I2C0_IRQHandler(void)
{
  // this can be locally defined
  I2C_TransferReturn_TypeDef transferStatus;
  // This shepherds the IC2 transfer along,
  // it’s a state machine! see em_i2c.c
  // It accesses global variables :
  // transferSequence
  // cmd_data
  // read_data
  // that we put into the data structure passed
  // to I2C_TransferInit()
  transferStatus = I2C_Transfer(I2C0);
  if (transferStatus == i2cTransferDone)
  {
    scheduler_setEvent_I2C_Transfer_Complete();
  }
  if (transferStatus < 0)
  {
    LOG_ERROR("I2C transaction failed with error code -> %d", transferStatus);
  }
} // I2C0_IRQHandler()

uint32_t letimerMilliseconds(void)
{
  return elapsed_ms * 3; //each tick is 3seconds
}
