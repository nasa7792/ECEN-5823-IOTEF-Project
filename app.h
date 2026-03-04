/***************************************************************************//**
 * @file
 * @brief Application interface provided to main().
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *
 * Editor: Feb 26, 2022, Dave Sluiter
 * Change: Added comment about use of .h files.
 *
 *
 *
 * Student edit: Add your name and email address here:
 * @student    Nalin Saxena, nasa7792@colorado.edu
 *
 *
 ******************************************************************************/

// Students: Remember, a header file (a .h file) defines an interface
//           for functions defined within an implementation file (a .c file).
//           The .h file defines what a caller (a user) of a .c file requires.
//           At a minimum, the .h file should define the publicly callable
//           functions, i.e. define the function prototypes. #define and type
//           definitions can be added if the caller requires theses.

#ifndef APP_H
#define APP_H
#include"src/i2c.h"
#include "src/scheduler.h"
#include "src/ble_device_type.h"
#include "src/ble.h"

#define LOWEST_ENERGY_MODE (2) // change this to switch b/w modes
#define LETIMER_PERIOD_MS (3030) //sensor sampling time as specified by A3
#define CONVERT_MS_TO_SEC (1000) //Conversion factor
#define CONVERT_US_TO_SEC (1000000) //Conversion factor


//declare all energy modes to be macros to avoid magic number
#define EM0_MODE (0)
#define EM1_MODE (1)
#define EM2_MODE (2)
#define EM3_MODE (3)

/* 
we need to conditionally define the macro for oscillator frequency so code remains uniform
*/
#if (LOWEST_ENERGY_MODE == EM3_MODE)
    #define FEQ_OSC (1000)
#else
  #define FEQ_OSC (32768)
#endif
//slow down the clock with the help of prescaler
#define PRE_SCALER_OSC (2)



#include"src/timer.h"
/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
void app_init(void);

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
void app_process_action(void);

#endif // APP_H
