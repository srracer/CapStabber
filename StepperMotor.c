/*
 * StepperMotor.c
 *
 *  Created on: Jun 17, 2017
 *      Author: slide
 */

#include "generic.h"
#include "StepperMotor.h"

// Drive screw mechanics



// GPIO Stepper Motor Assignments
// PA7: direction
// PF1: step pulse

#define DIRECTION_PERIPH SYSCTL_PERIPH_GPIOA
#define DIRECTION_PIN_BASE GPIO_PORTA_BASE
#define DIRECTION_PIN GPIO_PIN_7


#define STEP_PERIPH SYSCTL_PERIPH_GPIOF
#define STEP_PIN_BASE GPIO_PORTF_BASE
#define STEP_PIN GPIO_PIN_1


//Definitions
volatile uint32_t steps_to_move;
volatile float Z_Axis;
bool current_direction;

// Stepper Motor Functions
void mpu_init (void) {

	SysCtlPeripheralEnable(DIRECTION_PERIPH);
	SysCtlPeripheralEnable(STEP_PERIPH);
	SysCtlDelay(26); // wait for modules to start

	GPIOPinTypeGPIOOutput(DIRECTION_PIN_BASE, DIRECTION_PIN);
	GPIOPadConfigSet(DIRECTION_PIN_BASE, DIRECTION_PIN, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);

	GPIOPinTypeGPIOOutput(STEP_PIN_BASE, STEP_PIN);
	GPIOPadConfigSet(STEP_PIN_BASE, STEP_PIN, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);

}

void move_motor (float motor_speed, float number_of_mm, bool direction){

	uint32_t ui32Period;
	uint32_t number_of_steps = mm_to_steps(number_of_mm);

	if (steps_to_move>0) return;  //if motor is moving, then do not initiate a new move

	if (direction){
		GPIOPinWrite(DIRECTION_PIN_BASE, DIRECTION_PIN, GPIO_PIN_7);  //Set direction up
		SysCtlDelay(3);
	}
	else {
		GPIOPinWrite(DIRECTION_PIN_BASE, DIRECTION_PIN, 0);  //Set direction down
		SysCtlDelay(3);
	}

	current_direction = direction;
	steps_to_move = number_of_steps;

	ui32Period = (SysCtlClockGet() / (uint32_t)((5039*motor_speed) / 2));
	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period - 1);
	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();
	TimerEnable(TIMER0_BASE, TIMER_A);

}

void Timer0IntHandler(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	//Check if steps_to_move has been set to zero and if so, exit
	if (steps_to_move==0){
		TimerDisable(TIMER0_BASE, TIMER_A);
		return;
	}

	// Read the current state of the GPIO pin and
	// write back the opposite state
	if(GPIOPinRead(STEP_PIN_BASE, STEP_PIN))
	{
		GPIOPinWrite(STEP_PIN_BASE, STEP_PIN, 0);
	}
	else
	{
		GPIOPinWrite(STEP_PIN_BASE, STEP_PIN, STEP_PIN);
		steps_to_move--;  //adjust global to reflect moved 1 step
		if (current_direction){
			Z_Axis += (MM_PER_STEPS);
		}
		else {
			Z_Axis -= (MM_PER_STEPS);
		}
	}

	if (steps_to_move==0) TimerDisable(TIMER0_BASE, TIMER_A);
}

uint32_t mm_to_steps (float mm_of_travel) {return (uint32_t)((mm_of_travel*STEPS_PER_MM)+0.5);}

int32_t round (float x) {return (int)(x+0.5);}

