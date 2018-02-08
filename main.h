/*
 * main.h
 *
 *  Created on: Jun 17, 2017
 *      Author: slide
 */

#ifndef MAIN_H_
#define MAIN_H_




//*****************************************************************************
//
// The DMA control structure table.
//
//*****************************************************************************
#ifdef ewarm
#pragma data_alignment=1024
tDMAControlTable sDMAControlTable[64];
#elif defined(ccs)
#pragma DATA_ALIGN(sDMAControlTable, 1024)
tDMAControlTable sDMAControlTable[64];
#else
tDMAControlTable sDMAControlTable[64] __attribute__ ((aligned(1024)));
#endif

//*****************************************************************************
//
// Forward declarations for the globals required to define the widgets at
// compile-time.
//
//*****************************************************************************


//GUI Definitions and Functions
extern const uint8_t jmrlogo[];

void ClrScreen(void);
void DisplayLogo(void);
void Banner(void);

#endif /* MAIN_H_ */
