/*
 * MicroTimer.h
 *
 *  Created on: Jun 17, 2017
 *      Author: slide
 */

#ifndef MICROTIMER_H_
#define MICROTIMER_H_

//Declarations
extern volatile uint32_t micros;

//Timer functions
void SysTickInit(void);
void Wait(uint32_t);

#endif /* MICROTIMER_H_ */
