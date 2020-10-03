/*
 * CalibrateZ.c
 *
 *  Created on: Jul 23, 2017
 *      Author: slide
 */

#include "generic.h"
#include "LoadCell.h"
#include "MicroTimer.h"
#include "StepperMotor.h"

#include "WindowObjects.h"
#include "ReturnToMain.h"
#include "CalibrateZ.h"



//Calibrate Z Panel
// Step 1
Canvas(g_sCalZStep1, &g_psCalZPanel, 0, &g_CalZStep1Button, &g_sKentec320x240x16_SSD2119, 0, 30,
	       320, 40, (CANVAS_STYLE_FILL | CANVAS_STYLE_OUTLINE | CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_LEFT),
		   ClrWhite, ClrRed, ClrRed, &g_sFontCmss12, "Remove vial or weight, then...", 0, 0);
Canvas(g_sCalZStep1Result, &g_sCalZStep1, 0, 0, &g_sKentec320x240x16_SSD2119, 240, 35, 80, 30, CANVAS_STYLE_TEXT,
		   ClrWhite, 0, ClrRed, &g_sFontCmss12, 0, 0, 0);
RectangularButton(g_CalZStep1Button, &g_sCalZStep1, &g_sCalZStep1Result, 0, &g_sKentec320x240x16_SSD2119,
		170, 35, 65, 30, (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
        PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY), ClrSilver, ClrSilver, ClrRed, ClrBlue, &g_sFontCmss12b, "PRESS", 0, 0, 0, 0, OnCalZStep1Button);

// Step 2
Canvas(g_sCalZStep2, 0, 0, &g_CalZStep2Button, &g_sKentec320x240x16_SSD2119, 0, 80,
	       320, 40, (CANVAS_STYLE_FILL | CANVAS_STYLE_OUTLINE | CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_LEFT),
		   ClrWhite, ClrRed, ClrRed, &g_sFontCmss12, "Align tip of needle with shelf, then...", 0, 0);
Canvas(g_sCalZStep2Result, &g_sCalZStep2, 0, 0, &g_sKentec320x240x16_SSD2119, 240, 85, 80, 30, CANVAS_STYLE_TEXT,
		   ClrWhite, 0, ClrRed, &g_sFontCmss12, 0, 0, 0);
RectangularButton(g_CalZStep2Button, &g_sCalZStep2, &g_sCalZStep2Result, 0, &g_sKentec320x240x16_SSD2119,
		170, 85, 65, 30, (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
        PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY), ClrSilver, ClrSilver, ClrRed, ClrBlue, &g_sFontCmss12b, "PRESS", 0, 0, 0, 0, OnCalZStep2Button);


// Handles presses of the Calibrate Button
void
OnCalZButton(tWidget *pWidget)
{
	current_panel = 3;

	// Remove the Main Menu
	WidgetRemove((tWidget *)&g_psMainPanel);
	WidgetMessageQueueProcess();

    TurnOnReturnToMain();

    // Set the title of this panel.
    CanvasTextSet(&g_sTitle, "Calibrate Z Travel");
    WidgetPaint((tWidget *)&g_sTitle);
	WidgetMessageQueueProcess();

    // Add and draw the Cal Z Panel
    ResetCalZScreen();
	WidgetMessageQueueProcess();

    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_psCalZPanel);
    WidgetPaint((tWidget *)&g_psCalZPanel);
	WidgetMessageQueueProcess();

	//Execute Step 1
	// 1. Drive Z up by retract amount and speed (to ensure it is not pushing down on loadcell)
	move_motor(RETRACT_SPEED, CLEARANCE_HEIGHT, UP);

	while (steps_to_move>0){}  //wait for motor to stop moving
}


//Resets all Cal Screen Form controls
void ResetCalZScreen(void)
{
	//Step 1
	PushButtonTextSet(&g_CalZStep1Button, "PRESS");
	CanvasTextSet(&g_sCalZStep1Result, "");
	PushButtonCallbackSet(&g_CalZStep1Button, OnCalZStep1Button);
	CanvasTextOpaqueOff(&g_sCalZStep1Result);

	//Step 2
	WidgetRemove((tWidget *)&g_sCalZStep2);
	PushButtonTextSet(&g_CalZStep2Button, "PRESS");
	CanvasTextSet(&g_sCalZStep2Result, "");
	PushButtonCallbackSet(&g_CalZStep2Button, OnCalZStep2Button);
	CanvasTextOpaqueOff(&g_sCalZStep2Result);
}


void OnCalZStep1Button (tWidget *pWidget)
{
    // Change Text of Button
	PushButtonTextSet(&g_CalZStep1Button, "Wait..");
	PushButtonCallbackSet(&g_CalZStep1Button, 0);
    WidgetPaint((tWidget *)&g_CalZStep1Button);
	WidgetMessageQueueProcess();

	// 2. Measure loadcell - use this to subtract from reading to get zero
	int32_t unloaded_loadcell = hx711_average(5);

	// 3. Drive Z down slowly until change is measured in load cell
	move_motor(CAL_PLUNGE_SPEED, 50, DOWN);

	while(steps_to_move>0){
		//compute difference between reading and unloaded - trip at cal threshold, then, stop moving
		if ((scaled_reading(hx711_average(5))-scaled_reading(unloaded_loadcell))>Z_CAL_THRESHOLD) steps_to_move=0;
	}

	// 4. Establish this as Z=0
	Z_Axis = 0.0;

	// 5. Drive up to Puncture Depth to set correct needle extension
	move_motor(RETRACT_SPEED, (NEEDLE_PUNCTURE_DEPTH+CLEARANCE_HEIGHT), UP);

	while (steps_to_move>0){}  //wait for motor to stop moving

    // Change Text of Button
	PushButtonTextSet(&g_CalZStep1Button, "Done");
	PushButtonCallbackSet(&g_CalZStep1Button, 0);
    WidgetPaint((tWidget *)&g_CalZStep1Button);
    WidgetMessageQueueProcess();

	//add exception handling if value is out of expected range
	CanvasTextSet(&g_sCalZStep1Result, "...Success...");
	CanvasTextOpaqueOn(&g_sCalZStep1Result);
	WidgetPaint((tWidget *)&g_sCalZStep1Result);
	WidgetMessageQueueProcess();

    //Add Cal Step 2 to Widget Tree
	WidgetAdd((tWidget *)&g_psCalZPanel, (tWidget *)&g_sCalZStep2);
    WidgetPaint((tWidget *)&g_sCalZStep2);
	WidgetMessageQueueProcess();
}

void OnCalZStep2Button (tWidget *pWidget)
{
    // Change Text of Button
	PushButtonTextSet(&g_CalZStep2Button, "Wait..");
	PushButtonCallbackSet(&g_CalZStep1Button, 0);
    WidgetPaint((tWidget *)&g_CalZStep2Button);
	WidgetMessageQueueProcess();

	// Move Z up by retract height
	move_motor(RETRACT_SPEED, RETRACT_HEIGHT, UP);
	while (steps_to_move>0){}  //wait for motor to stop moving

    // Change Text of Button
	PushButtonTextSet(&g_CalZStep2Button, "Done");
	PushButtonCallbackSet(&g_CalZStep2Button, 0);
    WidgetPaint((tWidget *)&g_CalZStep2Button);
	WidgetMessageQueueProcess();

	//add exception handling if value is out of expected range
	CanvasTextSet(&g_sCalZStep2Result, "...Success...");
	CanvasTextOpaqueOn(&g_sCalZStep2Result);
    WidgetPaint((tWidget *)&g_sCalZStep2Result);
	WidgetMessageQueueProcess();

}





