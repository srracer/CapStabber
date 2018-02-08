
/*
 * LoadCell.c
 *
 *  Created on: Jun 17, 2017
 *      Author: slide
 */

#include "generic.h"
#include "LoadCell.h"
#include "MicroTimer.h"

// GPIO Load Cell Assignments
// Clock: PD6
// Data Output: PA3

#define HX711_DO_PERIPH SYSCTL_PERIPH_GPIOA
#define HX711_DO_BASE GPIO_PORTA_BASE
#define HX711_DO_PIN GPIO_PIN_3

#define HX711_CLK_PERIPH SYSCTL_PERIPH_GPIOD
#define HX711_CLK_BASE GPIO_PORTD_BASE
#define HX711_CLK_PIN GPIO_PIN_6

//Definitions
char weight_string[8];
int32_t LoadCell;
int32_t loadcell_cal_zero=0;
int32_t loadcell_cal_weight=1;
uint32_t cal_data_eeprom[2];  //unsigned for eeprom writing
uint32_t number_of_measurements=0;
float weight;

// Load Cell Functions
void hx711_init (void) {

	SysCtlPeripheralEnable(HX711_DO_PERIPH);
	SysCtlPeripheralEnable(HX711_CLK_PERIPH);
	SysCtlDelay(26); // wait for modules to start

	GPIOPinTypeGPIOInput(HX711_DO_BASE, HX711_DO_PIN);
	GPIOPadConfigSet(HX711_DO_BASE, HX711_DO_PIN, GPIO_STRENGTH_2MA , GPIO_PIN_TYPE_STD_WPD);

	GPIOPinTypeGPIOOutput(HX711_CLK_BASE, HX711_CLK_PIN);
	GPIOPadConfigSet(HX711_CLK_BASE, HX711_CLK_PIN, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);
}

void hx711_power_up(void)
{
	GPIOPinWrite(HX711_CLK_BASE, HX711_CLK_PIN, 0); //set low
}

void hx711_power_down(void)
{

	GPIOPinWrite(HX711_CLK_BASE, HX711_CLK_PIN, HX711_CLK_PIN); //set high
	Wait(61); //must be longer than 60 microseconds to send into power down mode.  device restarts at 128gain on ch. a
}

int32_t hx711_getvalue(void)
{
	int8_t i;
	uint32_t data=0;

	GPIOPinWrite(HX711_CLK_BASE, HX711_CLK_PIN, 0); //set low to power up

	while ((GPIOPinRead(HX711_DO_BASE, HX711_DO_PIN) & HX711_DO_PIN));  // Wait until Output pin goes low to start reading data
	for (i = 0; i<25; i++)
		{
			GPIOPinWrite(HX711_CLK_BASE, HX711_CLK_PIN, HX711_CLK_PIN);  // Set clock High - rising edge triggers data output
			if ((GPIOPinRead(HX711_DO_BASE, HX711_DO_PIN) & HX711_DO_PIN)){
				data |= 1 << (24-i);  //Evaluate output pin from hx711 - if high, write 1 to bit
			}
			else {
				data |= 0 << (24-i);   //else, write 0
			}
			GPIOPinWrite(HX711_CLK_BASE, HX711_CLK_PIN, 0);  // Set clock low
	}

	//Set gain to 128 by cycling clock pin 1 more time
	GPIOPinWrite(HX711_CLK_BASE, HX711_CLK_PIN, HX711_CLK_PIN);
	GPIOPinWrite(HX711_CLK_BASE, HX711_CLK_PIN, 0);

	GPIOPinWrite(HX711_CLK_BASE, HX711_CLK_PIN, HX711_CLK_PIN); //set high
	Wait(61); //Need to delay to shut off sensor

	data ^= 0x800000;  //2's complement

	return (int32_t)data;
}

int32_t hx711_average(int8_t samples)
{
	int32_t rolling_avg;
	int8_t i;

	rolling_avg = 0;

	for (i=0; i<samples; i++){
		rolling_avg += hx711_getvalue();
	}

	return rolling_avg/samples;
}

float scaled_reading(int32_t x)
{
  return  ((float)(CAL_WEIGHT)/((float)loadcell_cal_weight - (float)loadcell_cal_zero))*(((float)x) - (float)loadcell_cal_zero);
}

float convert_to_pounds(float grams)
{
	return grams*0.00220462;
}

