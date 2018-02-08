/*
 * main.c
 *
 *  Created on: Jun 17, 2017
 *      Author: slide
 */

#include "generic.h"
#include "main.h"
#include "LoadCell.h"
#include "MicroTimer.h"
#include "StepperMotor.h"

#include "WindowObjects.h"
#include "TestCapScreen.h"
#include "ReturnToMain.h"
#include "CalibrateScale.h"
#include "ScaleReading.h"
#include "CalibrateZ.h"

bool main_menu;
int current_panel;

tContext sContext;
tRectangle sRect;

//Main Menu Panel
//Each line represents an element in that panel
RectangularButton(g_CalZButton, &g_psMainPanel, 0, 0, &g_sKentec320x240x16_SSD2119,
		170, 85, 125, 40, (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY), ClrWhite, ClrWhite, ClrRed, ClrRed, &g_sFontCmss14, "Calibrate Z-Axis", 0, 0, 0, 0, OnCalZButton);
RectangularButton(g_ScaleReadingButton, &g_psMainPanel, &g_CalZButton, 0, &g_sKentec320x240x16_SSD2119,
		170, 135, 125, 40, (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY), ClrWhite, ClrWhite, ClrRed, ClrRed, &g_sFontCmss14, "Scale Reading", 0, 0, 0, 0, OnScaleReadingButton);
RectangularButton(g_TestConfigButton, &g_psMainPanel, &g_ScaleReadingButton, 0, &g_sKentec320x240x16_SSD2119,
		25, 85, 125, 40, (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY), ClrGray, ClrGray, ClrRed, ClrRed, &g_sFontCmss14, "Test Configuration", 0, 0, 0, 0, 0);
RectangularButton(g_CalibrateScaleButton, &g_psMainPanel, &g_TestConfigButton, 0, &g_sKentec320x240x16_SSD2119,
		170, 35, 125, 40, (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY), ClrWhite, ClrWhite, ClrRed, ClrRed, &g_sFontCmss14, "Calibrate Scale", 0, 0, 0, 0, OnCalibrateScaleButton);
RectangularButton(g_TestScreenButton, &g_psMainPanel, &g_CalibrateScaleButton, 0, &g_sKentec320x240x16_SSD2119,
		25, 35, 125, 40, (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY), ClrWhite, ClrWhite, ClrRed, ClrRed, &g_sFontCmss14, "Test Caps", 0, 0, 0, 0, OnTestScreenButton);


//Test Configuration Panel Screen
// List all widgets in the setup screen
// vial height, plunge depth, speed?, pass/fail threshold, save name?

// An set of canvas widgets, one per panel.  Each canvas is filled with
// black, overwriting the contents of the previous panel.  These are the parent widgets
Canvas(g_psMainPanel,0, 0, &g_TestScreenButton, &g_sKentec320x240x16_SSD2119, 0, 24,
		320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);
Canvas(g_psTestPanel,0, 0, &g_TestData, &g_sKentec320x240x16_SSD2119, 0, 24,
		320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);
Canvas(g_psCalibrateScalePanel,0, 0, &g_sCalibrationStep1, &g_sKentec320x240x16_SSD2119, 0, 24,
		320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);
Canvas(g_psScaleReadingPanel,0, 0, &g_sLoadCellValue, &g_sKentec320x240x16_SSD2119, 0, 24,
		320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);
Canvas(g_psCalZPanel,0, 0, &g_sCalZStep1, &g_sKentec320x240x16_SSD2119, 0, 24,
		320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);
//Canvas(g_psTestConfigPanel,0, 0, &g_sLoadCellValue, &g_sKentec320x240x16_SSD2119, 0, 24,
//		320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);

// The text and Return to Main button at the bottom of the screen.
Canvas(g_sTitle, 0, 0, 0, &g_sKentec320x240x16_SSD2119, 50, 200, 220, 40,
       CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_OPAQUE, 0, 0, ClrSilver,
       &g_sFontCmss20, 0, 0, 0);
RectangularButton(g_sReturnToMain, 0, 0, 0, &g_sKentec320x240x16_SSD2119, 255, 195,
                  60, 40, 0, ClrBlack, ClrBlack, ClrRed,
				  ClrRed, &g_sFontCmss12, "Main Menu", 0,
                  0, 0, 0, 0);

//Clears screen with black pixels
void ClrScreen()
{
   sRect.i16XMin = 0;
   sRect.i16YMin = 0;
   sRect.i16XMax = 319;
   sRect.i16YMax = 239;
   GrContextForegroundSet(&sContext, ClrBlack);
   GrRectFill(&sContext, &sRect);
   GrFlush(&sContext);
}

//Draws Jamar logo and waits for a delay
void DisplayLogo()
{
	ClrScreen();
	GrImageDraw(&sContext, jmrlogo, 0, 0);
//	SysCtlDelay(10000); // for debugging firmware work
	SysCtlDelay(SysCtlClockGet());  //  reinstate for prod release, but this delay is too long for debug purposes
	ClrScreen();
}

//Draws the banner across the top
void Banner()
{
    // Fill the top 24 rows of the screen with blue to create the banner.
    sRect.i16XMin = 0;
    sRect.i16YMin = 0;
    sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
    sRect.i16YMax = 23;
    GrContextForegroundSet(&sContext, ClrDarkBlue);
    GrRectFill(&sContext, &sRect);

    // Put a white box around the banner.
    GrContextForegroundSet(&sContext, ClrWhite);
    GrRectDraw(&sContext, &sRect);

    // Put the application name in the middle of the banner.
    GrContextFontSet(&sContext, &g_sFontCmss20b);
    GrStringDrawCentered(&sContext, "JAmar Puncture Force Tester", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 10, 0);
}



int main(void)

{
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); // 80Mhz -- 400/2/2.5

	//Initialize SysTick Interrupt
	SysTickInit();
	
	//Initialize load cell ports
	hx711_init();
	
	//Initialize stepper motor ports
	mpu_init();
	
	Kentec320x240x16_SSD2119Init(SysCtlClockGet());

	// The FPU should be enabled because some compilers will use floating-
	// point registers, even for non-floating-point code.  If the FPU is not
	// enabled this will cause a fault.  This also ensures that floating-
	// point operations could be added to this application and would work
	// correctly and use the hardware floating-point unit.  Finally, lazy
	// stacking is enabled for interrupt handlers.  This allows floating-
	// point instructions to be used within interrupt handlers, but at the
	// expense of extra stack usage.
	ROM_FPUEnable();
	ROM_FPULazyStackingEnable();

	// Configure and enable uDMA
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
	SysCtlDelay(26);
	uDMAControlBaseSet(&sDMAControlTable[0]);
	uDMAEnable();

	GrContextInit(&sContext, &g_sKentec320x240x16_SSD2119);

	// Initialize the touch screen driver and have it route its messages to the
	// widget tree.

	TouchScreenInit(SysCtlClockGet());
	TouchScreenCallbackSet(WidgetPointerMessage);

	//Enable EEPROM and Initialize
	SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
	EEPROMInit();

	//Write EEPROM data to cal vars
	EEPROMRead(cal_data_eeprom, 0x0, sizeof(cal_data_eeprom));

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

	//copy values to internal cal vars
	loadcell_cal_zero = (int32_t)cal_data_eeprom[0];
	loadcell_cal_weight = (int32_t)cal_data_eeprom[1];

	DisplayLogo();  // Flashes Jamar logo on screen at boot
	Banner(); // Places banner at top of screen

	// Add the title block
	WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sTitle);
	// Add the Main Menu Button
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sReturnToMain);

	// Add the first panel to the widget tree.
	WidgetAdd(WIDGET_ROOT, (tWidget *)&g_psMainPanel);
	CanvasTextSet(&g_sTitle, "      Main Menu      ");

	main_menu=TRUE;
	current_panel = 0;

	// Issue the initial paint request to the widgets.
	WidgetPaint(WIDGET_ROOT);

	// Loop forever handling widget messages.
	while (1)
	{
		// Process any messages in the widget message queue.
		WidgetMessageQueueProcess();
	}
}

