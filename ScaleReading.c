/*
 * ScaleReading.c
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
#include "ScaleReading.h"

void ResetInfoScreen(void);

//Scale Reading Panel Screen
Canvas(g_sLoadCellValue, &g_psScaleReadingPanel, 0, 0, &g_sKentec320x240x16_SSD2119, 20, 60,
	       280, 60, (CANVAS_STYLE_FILL | CANVAS_STYLE_OUTLINE | CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_HCENTER| CANVAS_STYLE_TEXT_VCENTER),
		   ClrWhite, ClrWhite, ClrRed, &g_sFontCmss16b, "Waiting for first measurement....", 0, 0);

// Handles presses of the Scale Reading Button
void
OnScaleReadingButton(tWidget *pWidget)
{
    int i;

    current_panel = 4;

	// Remove the Main Menu
    WidgetRemove((tWidget *)&g_psMainPanel);
	WidgetMessageQueueProcess();

    TurnOnReturnToMain();

    // Set the title of this panel.
    CanvasTextSet(&g_sTitle, "Scale Reading");
    WidgetPaint((tWidget *)&g_sTitle);
	WidgetMessageQueueProcess();

    // Add and draw the Information panel
    ResetInfoScreen();
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_psScaleReadingPanel);
    WidgetPaint((tWidget *)&g_psScaleReadingPanel);
	WidgetMessageQueueProcess();

	while (main_menu==FALSE)
	{

		LoadCell = hx711_average(5); // take sample(s) of load cell reading

		weight = scaled_reading(LoadCell);

		sprintf(weight_string,"%2.3flbs", weight*0.00220462);

     	CanvasTextSet(&g_sLoadCellValue, weight_string);
        WidgetPaint((tWidget *)&g_sLoadCellValue);

    	TouchScreenCallbackSet(WidgetPointerMessage);

    	WidgetMessageQueueProcess();

    	for (i=0; i<500; i++){
    		Wait(2);
    		WidgetMessageQueueProcess();
    		if (main_menu==TRUE) return;
    	}
    }

}

//Resets Info Screen Form controls
void ResetInfoScreen(void)
{
	CanvasTextSet(&g_sLoadCellValue, "Waiting for first measurement....");
}




