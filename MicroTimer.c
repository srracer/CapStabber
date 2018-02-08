/*
 * MicroTimer.c
 *
 *  Created on: Jun 17, 2017
 *      Author: slide
 */

#include "generic.h"
#include "MicroTimer.h"

void SysTickInt(void);

//Definitions
volatile uint32_t micros=0;

//When SysTickInt runs, increment micros
void SysTickInt(){
  micros++;
}

//Initialize and start SysTickInt to trigger every microsecond
void SysTickInit(){
  SysTickPeriodSet(80000000/1000000);  //F-CPU / 1000000
  SysTickIntRegister(SysTickInt);
  SysTickIntEnable();
  SysTickEnable();
}

//Wait uses systickint to delay precise number of microseconds
void Wait(uint32_t time){
	uint32_t temp = micros;
	while( (micros-temp) < time){
	}
}



