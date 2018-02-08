/*
 * StepperMotor.h
 *
 *  Created on: Jun 17, 2017
 *      Author: slide
 */

#ifndef STEPPERMOTOR_H_
#define STEPPERMOTOR_H_

// Stepper Motor

//Declarations
extern volatile uint32_t steps_to_move;
extern volatile float Z_Axis;
extern bool current_direction;

//Functions
void mpu_init (void);
void move_motor (float, float, bool);
void Timer0IntHandler(void);
uint32_t mm_to_steps (float);
int32_t round (float);

#endif /* STEPPERMOTOR_H_ */
