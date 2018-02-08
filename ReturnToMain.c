/*
 * ReturnToMain.c
 *
 *  Created on: Jun 18, 2017
 *      Author: slide
 */

#include "generic.h"
#include "ReturnToMain.h"
#include "MicroTimer.h"
#include "WindowObjects.h"
#include "StepperMotor.h"

void TurnOnReturnToMain(void)
{
	PushButtonCallbackSet(&g_sReturnToMain, OnReturnToMainButton);
	PushButtonOutlineOn(&g_sReturnToMain);
    PushButtonTextOn(&g_sReturnToMain);
    PushButtonFillOff(&g_sReturnToMain);
    WidgetPaint((tWidget *)&g_sReturnToMain);
	WidgetMessageQueueProcess();
    main_menu=FALSE;
}

void TurnOffReturnToMain(void)
{
	PushButtonCallbackSet(&g_sReturnToMain, 0);
	PushButtonOutlineOff(&g_sReturnToMain);
    PushButtonTextOff(&g_sReturnToMain);
    PushButtonFillOn(&g_sReturnToMain);
    WidgetPaint((tWidget *)&g_sReturnToMain);
	WidgetMessageQueueProcess();
}

// Handles presses of the Return To Main Button
void
OnReturnToMainButton(tWidget *pWidget)
{
	if (steps_to_move!=0) steps_to_move = 0;

    main_menu=TRUE;

    TurnOffReturnToMain();

	// Remove the previous screen Menu
    switch(current_panel){
    case 1:
    	WidgetRemove((tWidget *)&g_psTestPanel);
    	break;

    case 2:
    	WidgetRemove((tWidget *)&g_psCalibrateScalePanel);
    	break;

    case 3:
        WidgetRemove((tWidget *)&g_psCalZPanel);
        break;

    case 4:
        WidgetRemove((tWidget *)&g_psScaleReadingPanel);
        break;

    default:
    	break;

    }
	WidgetMessageQueueProcess();

	// Set the title of this panel.
	CanvasTextSet(&g_sTitle, "     Main Menu     ");
    WidgetPaint((tWidget *)&g_sTitle);
	WidgetMessageQueueProcess();

    // Add and draw the Main Menu panel
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_psMainPanel);
    WidgetPaint((tWidget *)&g_psMainPanel);
	WidgetMessageQueueProcess();
}


