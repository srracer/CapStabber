/*
 * generic.h
 *
 *  Created on: Jun 17, 2017
 *      Author: slide
 */

#ifndef GENERIC_H_
#define GENERIC_H_

//#define PART_TM4C123GH6PM

#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "time.h"
#include "stdlib.h"

#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_uart.h"
#include "inc/hw_pwm.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_adc.h"

#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/flash.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "driverlib/udma.h"
#include "driverlib/rom.h"
#include "driverlib/adc.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/ssi.h"
#include "driverlib/eeprom.h"

#include "grlib/grlib.h"
#include "grlib/widget.h"
#include "grlib/canvas.h"
#include "grlib/checkbox.h"
#include "grlib/container.h"
#include "grlib/pushbutton.h"
#include "grlib/radiobutton.h"
#include "grlib/slider.h"

#include "utils/ustdlib.h"
#include "Kentec320x240x16_ssd2119_spi.h"
#include "images.h"
#include "touch.h"

#include <string.h>

#define TRUE 1
#define FALSE 0

#define UP 1
#define DOWN 0

#define LEAD_PITCH .635
#define STEPS_PER_REV 3200.0
#define STEPS_PER_MM (STEPS_PER_REV/LEAD_PITCH)
#define MM_PER_STEPS (LEAD_PITCH/STEPS_PER_REV)

#define PASS_THRESHOLD 3.0 //lbs
#define Z_CAL_THRESHOLD 10 //grams

//all in mm
#define NEEDLE_PUNCTURE_DEPTH 9.0
#define CLEARANCE_HEIGHT 1.0
#define RETRACT_HEIGHT 2.0
#define TEST_PLUNGE_DEPTH (NEEDLE_PUNCTURE_DEPTH+RETRACT_HEIGHT)
#define SCALE_CAL_CLEARANCE 10.0

//all in mm/s
#define CAL_PLUNGE_SPEED 2.0
#define TEST_PLUNGE_SPEED 5.0
#define RETRACT_SPEED 10.0

extern bool main_menu;
extern int current_panel;
/*
	0 = Main
	1 = Test Panel
	2 = Calibrate Scale
	3 = Calibrate Z
	4 = Scale Reading
	5 = Test Configuration
*/


#endif /* GENERIC_H_ */
