/*
 * TestCapScreen.c
 *
 *  Created on: Jun 17, 2017
 *      Author: slide
 */

#include "generic.h"
#include "TestCapScreen.h"
#include "LoadCell.h"
#include "StepperMotor.h"
#include "MicroTimer.h"
#include "ReturnToMain.h"
#include "WindowObjects.h"

//Start Test Panel
//Test results text
Canvas(g_TestData, &g_psTestPanel, &g_TestResult, 0, &g_sKentec320x240x16_SSD2119, 10, 40,
	       215, 50, (CANVAS_STYLE_FILL | CANVAS_STYLE_OUTLINE | CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_HCENTER | CANVAS_STYLE_TEXT_VCENTER),
		   ClrBlack, ClrBlack, ClrRed, &g_sFontCmss14b, "Waiting to start test...", 0, 0);
//Results Text Box
Canvas(g_TestResult, &g_psTestPanel, 0, 0, &g_sKentec320x240x16_SSD2119, 20, 100,
	       175, 60, (CANVAS_STYLE_FILL | CANVAS_STYLE_OUTLINE | CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_HCENTER | CANVAS_STYLE_TEXT_VCENTER),
		   ClrBlack, ClrBlack, ClrWhite, &g_sFontCmss20b, "", 0, 0);

//Test button
RectangularButton(g_StartTestButton, 0,0,0, &g_sKentec320x240x16_SSD2119,
		230, 70, 80, 50, (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
        PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY), ClrWhite, ClrWhite, ClrRed, ClrRed, &g_sFontCmss12b, "Start Test", 0, 0, 0, 0, OnStartTestButton);

// Handles presses of the Start Test Button
void
OnTestScreenButton(tWidget *pWidget)
{
    current_panel = 1;

	// Remove the Main Menu
    WidgetRemove((tWidget *)&g_psMainPanel);
	WidgetMessageQueueProcess();

    TurnOnReturnToMain();

	// Set the title of this panel.
    CanvasTextSet(&g_sTitle, "   Test Screen   ");
    WidgetPaint((tWidget *)&g_sTitle);
	WidgetMessageQueueProcess();

    // Add and draw the Start Test panel
    ResetTestScreen();
    TurnOnStartTest();
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_psTestPanel);
    WidgetPaint((tWidget *)&g_psTestPanel);

    WidgetAdd((tWidget *)&g_psTestPanel, (tWidget *)&g_StartTestButton);
    WidgetPaint((tWidget *)&g_StartTestButton);
	WidgetMessageQueueProcess();

}

void TurnOnStartTest(void)
{
	PushButtonCallbackSet(&g_StartTestButton, OnStartTestButton);
	PushButtonOutlineOn(&g_StartTestButton);
    PushButtonTextOn(&g_StartTestButton);
    PushButtonFillColorSet(&g_StartTestButton, ClrWhite);
    PushButtonFillOn(&g_StartTestButton);
    WidgetPaint((tWidget *)&g_StartTestButton);
	WidgetMessageQueueProcess();
}


void TurnOffStartTest(void)
{
	PushButtonCallbackSet(&g_StartTestButton, 0);
	PushButtonOutlineOff(&g_StartTestButton);
    PushButtonTextOff(&g_StartTestButton);
    PushButtonFillColorSet(&g_StartTestButton, ClrBlack);
    PushButtonFillOn(&g_StartTestButton);
    WidgetPaint((tWidget *)&g_StartTestButton);
	WidgetMessageQueueProcess();
}

void TestPass(void)
{
	CanvasFillColorSet(&g_TestResult, ClrGreen);
	CanvasTextSet(&g_TestResult, "PASS");
    WidgetPaint((tWidget *)&g_TestResult);
	WidgetMessageQueueProcess();
}

void TestFail(void)
{
	CanvasFillColorSet(&g_TestResult, ClrRed);
	CanvasTextSet(&g_TestResult, "FAIL");
    WidgetPaint((tWidget *)&g_TestResult);
	WidgetMessageQueueProcess();
}

void OnStartTestButton (tWidget *pWidget)
{
	float peak_loadcell_reading = 0.0;
//	float peak_loadcell_z = 0.0;
	char result[40];

	// Disable Test and return to main buttons during test
	TurnOffStartTest();
	TurnOffReturnToMain();

	//Start Driving Down with measurements

	CanvasTextSet(&g_TestData, "...Moving Down...");
	WidgetPaint((tWidget *)&g_TestData);
	WidgetMessageQueueProcess();

	//If this is first test after a scale cal, then drive will be high - need to first move down to correct height
	if (Z_Axis>(NEEDLE_PUNCTURE_DEPTH+CLEARANCE_HEIGHT+RETRACT_HEIGHT)){
		move_motor(RETRACT_SPEED, (Z_Axis-(NEEDLE_PUNCTURE_DEPTH+CLEARANCE_HEIGHT+RETRACT_HEIGHT)),DOWN);
	}

	//MAY NEED TO ADD ERROR HANDLING IF Z is off?

	move_motor(TEST_PLUNGE_SPEED, TEST_PLUNGE_DEPTH, DOWN);

	while(steps_to_move>0){

		// Make Measurement and write to peak if highest
		LoadCell = hx711_average(1); // take 1 sample for fast timing
		weight = scaled_reading(LoadCell);
		if (weight>peak_loadcell_reading){
			peak_loadcell_reading = weight;
//			peak_loadcell_z = Z_Axis;
		}
	}

	CanvasTextSet(&g_TestData, "...Moving Up...");
	WidgetPaint((tWidget *)&g_TestData);
	WidgetMessageQueueProcess();

	move_motor(RETRACT_SPEED, TEST_PLUNGE_DEPTH, UP);

	while (steps_to_move>0){}  //wait for motor to stop moving

	// Display test result - add logic to determine pass/fail
	sprintf(result,"%2.3flbs", convert_to_pounds(peak_loadcell_reading));

	if (convert_to_pounds(peak_loadcell_reading)>PASS_THRESHOLD) TestFail();

	else if (convert_to_pounds(peak_loadcell_reading)<=PASS_THRESHOLD)	TestPass();

	CanvasTextSet(&g_TestData, result);

    WidgetPaint((tWidget *)&g_TestData);
	WidgetMessageQueueProcess();

    //Enable Test button with new text and return to main
	TurnOnStartTest();
	TurnOnReturnToMain();
	PushButtonTextSet(&g_StartTestButton, "Test Another");
	WidgetPaint((tWidget *)&g_StartTestButton);
	WidgetMessageQueueProcess();
}

//Resets all Test Screen Form controls
void ResetTestScreen(void)
{
	CanvasFillColorSet(&g_TestResult, ClrBlack);
	CanvasTextSet(&g_TestResult, "");
	PushButtonTextSet(&g_StartTestButton, "Start Test");
	PushButtonCallbackSet(&g_StartTestButton, OnStartTestButton);
	CanvasTextSet(&g_TestData, "Waiting to start test...");
	WidgetMessageQueueProcess();
}
