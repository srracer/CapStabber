/*
 * TestCapScreen.h
 *
 *  Created on: Jun 17, 2017
 *      Author: slide
 */

#ifndef TESTCAPSCREEN_H_
#define TESTCAPSCREEN_H_



void OnStartTestButton(tWidget *pWidget);
void OnTestScreenButton(tWidget *pWidget);

void TurnOnStartTest(void);
void TurnOffStartTest(void);
void ResetTestScreen(void);
void TestPass(void);
void TestFail(void);

#endif /* TESTCAPSCREEN_H_ */
