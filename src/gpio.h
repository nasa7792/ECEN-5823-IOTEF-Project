/*
   gpio.h
  
    Created on: Dec 12, 2018
        Author: Dan Walkes

    Updated by Dave Sluiter Sept 7, 2020. moved #defines from .c to .h file.
    Updated by Dave Sluiter Dec 31, 2020. Minor edits with #defines.

    Editor: Feb 26, 2022, Dave Sluiter
    Change: Added comment about use of .h files.

 *
 * Student edit: Add your name and email address here:
 * @student    Nalin Saxena, nasa7792@colorado.edu
 *
 
 */


// Students: Remember, a header file (a .h file) generally defines an interface
//           for functions defined within an implementation file (a .c file).
//           The .h file defines what a caller (a user) of a .c file requires.
//           At a minimum, the .h file should define the publicly callable
//           functions, i.e. define the function prototypes. #define and type
//           definitions can be added if the caller requires theses.


#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_
#include"stdbool.h"

//After reviewing https://www.silabs.com/documents/public/user-guides/ug279-brd4104a-user-guide.pdf
#define LED_port   (gpioPortF)
#define TEMP_SENSOR_PORT (gpioPortD)
#define LED0_pin   (4) //UIF_LED0
#define LED1_pin   (5) //UIF_LED1
#define USR_BTN0   (6)//PF6
#define TEMP_SENSOR_ENABLE_PIN (15)
#define DISP_EXTCOMIN (13)



// Function prototypes
void gpioInit();
void gpioLed0SetOn();
void gpioLed0SetOff();
void gpioLed1SetOn();
void gpioLed1SetOff();

/*
turns on the gpio enable assosicated with Si7021
*/
void enable_display();
/*
turns off the gpio enable assosicated with Si7021
*/
void disable_display();


void gpioSetDisplayExtcomin(bool value);

#endif /* SRC_GPIO_H_ */
