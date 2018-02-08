/*
 * CalibrateScale.c
 *
 *  Created on: Jun 18, 2017
 *      Author: slide
 */

#include "generic.h"
#include "LoadCell.h"
#include "MicroTimer.h"
#include "StepperMotor.h"

#include "WindowObjects.h"
#include "ReturnToMain.h"
#include "CalibrateScale.h"

//Calibrate Scale Panel
// Step 1
Canvas(g_sCalibrationStep1, &g_psCalibrateScalePanel, 0, &g_CalStep1Button, &g_sKentec320x240x16_SSD2119, 0, 30,
	       320, 40, (CANVAS_STYLE_FILL | CANVAS_STYLE_OUTLINE | CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_LEFT),
		   ClrWhite, ClrRed, ClrRed, &g_sFontCmss12, "Remove vial or weight, then...", 0, 0);
Canvas(g_sCalStep1Result, &g_sCalibrationStep1, 0, 0, &g_sKentec320x240x16_SSD2119, 240, 35, 80, 30, CANVAS_STYLE_TEXT,
		   ClrWhite, 0, ClrRed, &g_sFontCmss12, 0, 0, 0);
RectangularButton(g_CalStep1Button, &g_sCalibrationStep1, &g_sCalStep1Result, 0, &g_sKentec320x240x16_SSD2119,
		170, 35, 65, 30, (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
        PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY), ClrSilver, ClrSilver, ClrRed, ClrBlue, &g_sFontCmss12b, "PRESS", 0, 0, 0, 0, OnCalStep1Button);

//Calibrate Scale Panel continued
// Step 2
Canvas(g_sCalibrationStep2, 0, 0, &g_CalStep2Button, &g_sKentec320x240x16_SSD2119, 0, 80,
	       320, 40, (CANVAS_STYLE_FILL | CANVAS_STYLE_OUTLINE | CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_LEFT),
		   ClrWhite, ClrRed, ClrRed, &g_sFontCmss12, "Place calibration weight, then...", 0, 0);
Canvas(g_sCalStep2Result, &g_sCalibrationStep2, 0, 0, &g_sKentec320x240x16_SSD2119, 240, 85, 80, 30, CANVAS_STYLE_TEXT,
		   ClrWhite, 0, ClrRed, &g_sFontCmss12, 0, 0, 0);
RectangularButton(g_CalStep2Button, &g_sCalibrationStep2, &g_sCalStep2Result, 0, &g_sKentec320x240x16_SSD2119,
		170, 85, 65, 30, (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
        PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY), ClrSilver, ClrSilver, ClrRed, ClrBlue, &g_sFontCmss12b, "PRESS", 0, 0, 0, 0, OnCalStep2Button);


// Handles presses of the Calibrate Button
void
OnCalibrateScaleButton(tWidget *pWidget)
{
    current_panel = 2;

	// Remove the Main Menu
    WidgetRemove((tWidget *)&g_psMainPanel);
	WidgetMessageQueueProcess();

    TurnOnReturnToMain();

    // Set the title of this panel.
    CanvasTextSet(&g_sTitle, "  Calibrate Scale ");
    WidgetPaint((tWidget *)&g_sTitle);
	WidgetMessageQueueProcess();

    // Add and draw the Start Test panel
    ResetCalScreen();
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_psCalibrateScalePanel);
    WidgetPaint((tWidget *)&g_psCalibrateScalePanel);
	WidgetMessageQueueProcess();

	//Retract needle for cal weight clearance
	move_motor(RETRACT_SPEED, SCALE_CAL_CLEARANCE, UP);

	while (steps_to_move>0){}  //wait for motor to stop moving
}


//Resets all Cal Screen Form controls
void ResetCalScreen(void)
{
	//Step 1
	PushButtonTextSet(&g_CalStep1Button, "PRESS");
	CanvasTextSet(&g_sCalStep1Result, "");
	PushButtonCallbackSet(&g_CalStep1Button, OnCalStep1Button);
	CanvasTextOpaqueOff(&g_sCalStep1Result);

	//Step 2
	WidgetRemove((tWidget *)&g_sCalibrationStep2);
	PushButtonTextSet(&g_CalStep2Button, "PRESS");
	CanvasTextSet(&g_sCalStep2Result, "");
	PushButtonCallbackSet(&g_CalStep2Button, OnCalStep2Button);
	CanvasTextOpaqueOff(&g_sCalStep2Result);
}


void OnCalStep1Button (tWidget *pWidget)
{
    // Change Text of Button
	PushButtonTextSet(&g_CalStep1Button, "Wait..");
	PushButtonCallbackSet(&g_CalStep1Button, 0);
    WidgetPaint((tWidget *)&g_CalStep1Button);
	WidgetMessageQueueProcess();

	//Measure sensor here - assign to 0 weight
	loadcell_cal_zero = hx711_average(CAL_MEASUREMENTS); // take samples of load cell reading
	cal_data_eeprom[0] = (uint32_t)loadcell_cal_zero;				// copy to eeprom array

    // Change Text of Button
	PushButtonTextSet(&g_CalStep1Button, "Done");
	PushButtonCallbackSet(&g_CalStep1Button, 0);
    WidgetPaint((tWidget *)&g_CalStep1Button);
    WidgetMessageQueueProcess();

	//add exception handling if value is out of expected range
	CanvasTextSet(&g_sCalStep1Result, "...Success...");
	CanvasTextOpaqueOn(&g_sCalStep1Result);
	WidgetPaint((tWidget *)&g_sCalStep1Result);
	WidgetMessageQueueProcess();

    //Add Cal Step 2 to Widget Tree
	WidgetAdd((tWidget *)&g_psCalibrateScalePanel, (tWidget *)&g_sCalibrationStep2);
    WidgetPaint((tWidget *)&g_sCalibrationStep2);
	WidgetMessageQueueProcess();
}

void OnCalStep2Button (tWidget *pWidget)
{
    // Change Text of Button
	PushButtonTextSet(&g_CalStep2Button, "Wait..");
	PushButtonCallbackSet(&g_CalStep1Button, 0);
    WidgetPaint((tWidget *)&g_CalStep2Button);
	WidgetMessageQueueProcess();

	//Measure sensor here - assign to cal weight
	loadcell_cal_weight = hx711_average(CAL_MEASUREMENTS);
	cal_data_eeprom[1] = (uint32_t)loadcell_cal_weight;

	EEPROMProgram(cal_data_eeprom, 0x0, sizeof(cal_data_eeprom));

    // Change Text of Button
	PushButtonTextSet(&g_CalStep2Button, "Done");
	PushButtonCallbackSet(&g_CalStep2Button, 0);
    WidgetPaint((tWidget *)&g_CalStep2Button);
	WidgetMessageQueueProcess();

	//add exception handling if value is out of expected range
	CanvasTextSet(&g_sCalStep2Result, "...Success...");
	CanvasTextOpaqueOn(&g_sCalStep2Result);
    WidgetPaint((tWidget *)&g_sCalStep2Result);
	WidgetMessageQueueProcess();

}


