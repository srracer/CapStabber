//#define PART_TM4C123GH6PM

#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "time.h"
#include "stdlib.h"

#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_uart.h"
#include "inc/hw_pwm.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_adc.h"

#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/flash.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "driverlib/udma.h"
#include "driverlib/rom.h"
#include "driverlib/adc.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/ssi.h"
#include "driverlib/eeprom.h"

#include "grlib/grlib.h"
#include "grlib/widget.h"
#include "grlib/canvas.h"
#include "grlib/checkbox.h"
#include "grlib/container.h"
#include "grlib/pushbutton.h"
#include "grlib/radiobutton.h"
#include "grlib/slider.h"

#include "utils/ustdlib.h"
#include "Kentec320x240x16_ssd2119_spi.h"
#include "images.h"
#include "touch.h"

#include <string.h>

#define TRUE 1
#define FALSE 0

//Timer functions
volatile uint32_t micros=0;

void SysTickInt(){
  micros++;
}
void SysTickInit(){
  SysTickPeriodSet(80000000/1000000);  //F-CPU / 1000000
  SysTickIntRegister(SysTickInt);
  SysTickIntEnable();
  SysTickEnable();
}

void Wait(uint32_t time){
	uint32_t temp = micros;
	while( (micros-temp) < time){
	}
}

//Load Cell Settings

// GPIO Load Cell Assignments
// Clock: PD6
// Data Output: PA3

#define HX711_DO_PERIPH SYSCTL_PERIPH_GPIOA
#define HX711_DO_BASE GPIO_PORTA_BASE
#define HX711_DO_PIN GPIO_PIN_3

#define HX711_CLK_PERIPH SYSCTL_PERIPH_GPIOD
#define HX711_CLK_BASE GPIO_PORTD_BASE
#define HX711_CLK_PIN GPIO_PIN_6

#define CAL_WEIGHT 200                // build in 200g into code for calibration weight
#define CAL_MEASUREMENTS 20

// These variables are used to store the Load Cell measurement data
char weight_string[8];

int32_t LoadCell;
int32_t loadcell_cal_zero=0;
int32_t loadcell_cal_weight=1;
uint32_t cal_data_eeprom[2];  //unsigned for eeprom writing

float weight;

float scaled_reading(int32_t x)
{
  return  ((float)(CAL_WEIGHT)/((float)loadcell_cal_weight - (float)loadcell_cal_zero))*(((float)x) - (float)loadcell_cal_zero);
}

//Load Cell functions
int32_t hx711_getvalue(void);
int32_t hx711_average(int8_t samples);
void hx711_power_down(void);
void hx711_power_up(void);

// Stepper Motor
// Drive screw mechanics

#define LEAD_PITCH .635
#define STEPS_PER_REV 3200
#define STEPS_PER_MM (STEPS_PER_REV/LEAD_PITCH)

// GPIO Stepper Motor Assignments
// PA7: direction
// PF1: step pulse

#define DIRECTION_PERIPH SYSCTL_PERIPH_GPIOA
#define DIRECTION_PIN_BASE GPIO_PORTA_BASE
#define DIRECTION_PIN GPIO_PIN_7
#define DOWN 0
#define UP 1

#define STEP_PERIPH SYSCTL_PERIPH_GPIOF
#define STEP_PIN_BASE GPIO_PORTF_BASE
#define STEP_PIN GPIO_PIN_1

uint32_t mm_to_steps (float mm_of_travel) {return (uint32_t)((mm_of_travel*STEPS_PER_MM)+0.5);}
int32_t round (float x) {return (int)(x+0.5);}

volatile uint32_t steps_to_move = 0;

// Load Cell Functions
void hx711_init (void) {

	SysCtlPeripheralEnable(HX711_DO_PERIPH);
	SysCtlPeripheralEnable(HX711_CLK_PERIPH);
	SysCtlDelay(26); // wait for modules to start

	GPIOPinTypeGPIOInput(HX711_DO_BASE, HX711_DO_PIN);
	GPIOPadConfigSet(HX711_DO_BASE, HX711_DO_PIN, GPIO_STRENGTH_2MA , GPIO_PIN_TYPE_STD_WPD);

	GPIOPinTypeGPIOOutput(HX711_CLK_BASE, HX711_CLK_PIN);
	GPIOPadConfigSet(HX711_CLK_BASE, HX711_CLK_PIN, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);
}

void hx711_power_up(void)
{
	GPIOPinWrite(HX711_CLK_BASE, HX711_CLK_PIN, 0);
}

void hx711_power_down(void)
{

	GPIOPinWrite(HX711_CLK_BASE, HX711_CLK_PIN, HX711_CLK_PIN);
	Wait(62); //must be longer than 60 microseconds to send into power down mode.  device restarts at 128gain on ch. a
}

int32_t hx711_getvalue(void)
{
	int8_t i;
	uint32_t data=0;

	hx711_power_up();

	while ((GPIOPinRead(HX711_DO_BASE, HX711_DO_PIN) & HX711_DO_PIN));  // Wait until Output pin goes low to start reading data
	for (i = 0; i<25; i++)
		{
			GPIOPinWrite(HX711_CLK_BASE, HX711_CLK_PIN, HX711_CLK_PIN);  // Set clock High - rising edge triggers data output
			if ((GPIOPinRead(HX711_DO_BASE, HX711_DO_PIN) & HX711_DO_PIN)){
				data |= 1 << (24-i);  //Evaluate output pin from hx711 - if high, write 1 to bit
			}
			else {
				data |= 0 << (24-i);   //else, write 0
			}
			GPIOPinWrite(HX711_CLK_BASE, HX711_CLK_PIN, 0);  // Set clock low
	}

	//Set gain to 128 by cycling clock pin 1 more time
	GPIOPinWrite(HX711_CLK_BASE, HX711_CLK_PIN, HX711_CLK_PIN);
	GPIOPinWrite(HX711_CLK_BASE, HX711_CLK_PIN, 0);

	hx711_power_down();

	data ^= 0x800000;  //2's complement

	return (int32_t)data;
}

int32_t hx711_average(int8_t samples)
{
	int32_t rolling_avg;
	int8_t i;

	rolling_avg = 0.0;

	for (i=0; i<samples; i++){
		rolling_avg += hx711_getvalue();
	}

	return rolling_avg/samples;
}

// Stepper Motor Functions
void mpu_init (void) {

	SysCtlPeripheralEnable(DIRECTION_PERIPH);
	SysCtlPeripheralEnable(STEP_PERIPH);
	SysCtlDelay(26); // wait for modules to start

	GPIOPinTypeGPIOOutput(DIRECTION_PIN_BASE, DIRECTION_PIN);
	GPIOPadConfigSet(DIRECTION_PIN_BASE, DIRECTION_PIN, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);

	GPIOPinTypeGPIOOutput(STEP_PIN_BASE, STEP_PIN);
	GPIOPadConfigSet(STEP_PIN_BASE, STEP_PIN, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);

}

void move_motor (float motor_speed, uint32_t number_of_steps, bool direction){

	uint32_t ui32Period;

	if (steps_to_move>0) return;  //if motor is moving, then do not initiate a new move

	if (direction){
		GPIOPinWrite(DIRECTION_PIN_BASE, DIRECTION_PIN, GPIO_PIN_7);  //Set direction up
		SysCtlDelay(3);
	}
	else {
		GPIOPinWrite(DIRECTION_PIN_BASE, DIRECTION_PIN, 0);  //Set direction down
		SysCtlDelay(3);
	}

	steps_to_move = number_of_steps;

	ui32Period = (SysCtlClockGet() / (uint32_t)((5039*motor_speed) / 2));
	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period - 1);
	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();
	TimerEnable(TIMER0_BASE, TIMER_A);

}

void Timer0IntHandler(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	// Read the current state of the GPIO pin and
	// write back the opposite state
	if(GPIOPinRead(STEP_PIN_BASE, STEP_PIN))
	{
		GPIOPinWrite(STEP_PIN_BASE, STEP_PIN, 0);
	}
	else
	{
		GPIOPinWrite(STEP_PIN_BASE, STEP_PIN, STEP_PIN);
		steps_to_move--;  //adjust global to reflect moved 1 step
	}

	if (steps_to_move==0) TimerDisable(TIMER0_BASE, TIMER_A);

}

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

tContext sContext;
tRectangle sRect;

void ClrScreen(void);
void DisplayLogo(void);
void Banner(void);
void TurnOffReturnToMain(void);
void TurnOnReturnToMain(void);
void TurnOffStartTest(void);
void TurnOnStartTest(void);
void ResetCalScreen(void);
void ResetTestScreen(void);

extern tCanvasWidget g_psMainPanel;
extern tCanvasWidget g_psTestPanel;
extern tCanvasWidget g_psCalibrationPanel;
extern tCanvasWidget g_psInfoPanel;
extern tPushButtonWidget g_CalStep1Button;
extern tPushButtonWidget g_CalStep2Button;
extern tPushButtonWidget g_CalStep3Button;
extern tPushButtonWidget g_StartTestButton;

void OnTestScreenButton(tWidget *pWidget);
void OnStartTestButton(tWidget *pWidget);
void OnCalibrateButton(tWidget *pWidget);
void OnInfoButton(tWidget *pWidget);
void OnReturnToMainButton(tWidget *pWidget);
void OnCalStep1Button(tWidget *pWidget);
void OnCalStep2Button(tWidget *pWidget);
void OnCalStep3Button(tWidget *pWidget);

bool main_menu=TRUE;

//Main Menu Panel
RectangularButton(g_InfoButton, &g_psMainPanel, 0, 0, &g_sKentec320x240x16_SSD2119,
		170, 100, 125, 50, (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY), ClrWhite, ClrWhite, ClrRed, ClrRed, &g_sFontCmss14, "Information", 0, 0, 0, 0, OnInfoButton);
RectangularButton(g_SetupScreenButton, &g_psMainPanel, &g_InfoButton, 0, &g_sKentec320x240x16_SSD2119,
		25, 100, 125, 50, (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY), ClrWhite, ClrWhite, ClrRed, ClrRed, &g_sFontCmss14, "Setup Screen", 0, 0, 0, 0, 0);
RectangularButton(g_StartCalibrationButton, &g_psMainPanel, &g_SetupScreenButton, 0, &g_sKentec320x240x16_SSD2119,
		170, 35, 125, 50, (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY), ClrWhite, ClrWhite, ClrRed, ClrRed, &g_sFontCmss14, "Calibrate Screen", 0, 0, 0, 0, OnCalibrateButton);
RectangularButton(g_TestScreenButton, &g_psMainPanel, &g_StartCalibrationButton, 0, &g_sKentec320x240x16_SSD2119,
		25, 35, 125, 50, (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY), ClrWhite, ClrWhite, ClrRed, ClrRed, &g_sFontCmss14, "Test Screen", 0, 0, 0, 0, OnTestScreenButton);

//Start Test Panel
//Test results text
Canvas(g_TestingResults, &g_psTestPanel, 0, 0, &g_sKentec320x240x16_SSD2119, 0, 80,
	       225, 40, (CANVAS_STYLE_FILL | CANVAS_STYLE_OUTLINE | CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_LEFT),
		   ClrBlack, ClrBlack, ClrRed, &g_sFontCmss12, "Waiting to start test...", 0, 0);
//Test button
RectangularButton(g_StartTestButton, 0,0,0, &g_sKentec320x240x16_SSD2119,
		230, 70, 80, 50, (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
        PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY), ClrWhite, ClrWhite, ClrRed, ClrRed, &g_sFontCmss12, "Start Test", 0, 0, 0, 0, OnStartTestButton);

//Calibrate Panel
// Step 1
Canvas(g_sCalibrationStep1, &g_psCalibrationPanel, 0, &g_CalStep1Button, &g_sKentec320x240x16_SSD2119, 0, 30,
	       320, 40, (CANVAS_STYLE_FILL | CANVAS_STYLE_OUTLINE | CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_LEFT),
		   ClrWhite, ClrRed, ClrRed, &g_sFontCmss12, "Remove vials and weights, then...", 0, 0);
Canvas(g_sCalStep1Result, &g_sCalibrationStep1, 0, 0, &g_sKentec320x240x16_SSD2119, 240, 35, 80, 30, CANVAS_STYLE_TEXT,
		   ClrWhite, 0, ClrRed, &g_sFontCmss12, 0, 0, 0);
RectangularButton(g_CalStep1Button, &g_sCalibrationStep1, &g_sCalStep1Result, 0, &g_sKentec320x240x16_SSD2119,
		170, 35, 65, 30, (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
        PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY), ClrSilver, ClrSilver, ClrRed, ClrBlue, &g_sFontCmss12b, "PRESS", 0, 0, 0, 0, OnCalStep1Button);

// Step 2
Canvas(g_sCalibrationStep2, 0, 0, &g_CalStep2Button, &g_sKentec320x240x16_SSD2119, 0, 80,
	       320, 40, (CANVAS_STYLE_FILL | CANVAS_STYLE_OUTLINE | CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_LEFT),
		   ClrWhite, ClrRed, ClrRed, &g_sFontCmss12, "Place 200g weight, then...", 0, 0);
Canvas(g_sCalStep2Result, &g_sCalibrationStep2, 0, 0, &g_sKentec320x240x16_SSD2119, 240, 85, 80, 30, CANVAS_STYLE_TEXT,
		   ClrWhite, 0, ClrRed, &g_sFontCmss12, 0, 0, 0);
RectangularButton(g_CalStep2Button, &g_sCalibrationStep2, &g_sCalStep2Result, 0, &g_sKentec320x240x16_SSD2119,
		170, 85, 65, 30, (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
        PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY), ClrSilver, ClrSilver, ClrRed, ClrBlue, &g_sFontCmss12b, "PRESS", 0, 0, 0, 0, OnCalStep2Button);

//Information Screeen
Canvas(g_sLoadCellValue, &g_psInfoPanel, 0, 0, &g_sKentec320x240x16_SSD2119, 0, 30,
	       320, 40, (CANVAS_STYLE_FILL | CANVAS_STYLE_OUTLINE | CANVAS_STYLE_TEXT | CANVAS_STYLE_TEXT_LEFT),
		   ClrWhite, ClrRed, ClrRed, &g_sFontCmss12, "Waiting for first measurement....", 0, 0);

// An set of canvas widgets, one per panel.  Each canvas is filled with
// black, overwriting the contents of the previous panel.  These are the parent widgets
Canvas(g_psMainPanel,0, 0, &g_TestScreenButton, &g_sKentec320x240x16_SSD2119, 0, 24,
		320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);
Canvas(g_psTestPanel,0, 0, &g_TestingResults, &g_sKentec320x240x16_SSD2119, 0, 24,
		320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);
Canvas(g_psCalibrationPanel,0, 0, &g_sCalibrationStep1, &g_sKentec320x240x16_SSD2119, 0, 24,
		320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);
Canvas(g_psInfoPanel,0, 0, &g_sLoadCellValue, &g_sKentec320x240x16_SSD2119, 0, 24,
		320, 166, CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);

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
	SysCtlDelay(10000);
//	SysCtlDelay(SysCtlClockGet());  reinstate after debug, but this delay is too long for debug purposes
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

//Resets all Test Screen Form controls
void ResetTestScreen(void)
{
	PushButtonTextSet(&g_StartTestButton, "Start Test");
	PushButtonCallbackSet(&g_StartTestButton, OnStartTestButton);
	CanvasTextSet(&g_TestingResults, "Waiting to start test...");
}

//Resets Info Screen Form controls
void ResetInfoScreen(void)
{
	CanvasTextSet(&g_sLoadCellValue, "Waiting for first measurement....");
}

// Handles presses of the Start Test Button
void
OnTestScreenButton(tWidget *pWidget)
{
    // Set the title of this panel.
    CanvasTextSet(&g_sTitle, "   Test Screen   ");
    WidgetPaint((tWidget *)&g_sTitle);

    TurnOnReturnToMain();

	// Remove the Main Menu
    WidgetRemove((tWidget *)&g_psMainPanel);

    // Add and draw the Start Test panel
    ResetTestScreen();
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_psTestPanel);
    WidgetPaint((tWidget *)&g_psTestPanel);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_StartTestButton);
    WidgetPaint((tWidget *)&g_StartTestButton);

}

// Handles presses of the Calibrate Button
void
OnCalibrateButton(tWidget *pWidget)
{
    // Set the title of this panel.
    CanvasTextSet(&g_sTitle, "   Calibration   ");
    WidgetPaint((tWidget *)&g_sTitle);

    TurnOnReturnToMain();

	// Remove the Main Menu
    WidgetRemove((tWidget *)&g_psMainPanel);

    // Add and draw the Start Test panel
    ResetCalScreen();
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_psCalibrationPanel);
    WidgetPaint((tWidget *)&g_psCalibrationPanel);
}


// Handles presses of the Information Button
void
OnInfoButton(tWidget *pWidget)
{
    // Set the title of this panel.
    CanvasTextSet(&g_sTitle, "   Information   ");
    WidgetPaint((tWidget *)&g_sTitle);

	// Remove the Main Menu
    WidgetRemove((tWidget *)&g_psMainPanel);
    TurnOnReturnToMain();

    // Add and draw the Information panel
    ResetInfoScreen();
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_psInfoPanel);
    WidgetPaint((tWidget *)&g_psInfoPanel);
	WidgetMessageQueueProcess();

	while (main_menu==FALSE)
	{

		LoadCell = hx711_average(5); // take 5 samples of load cell reading

		weight = scaled_reading(LoadCell);

		sprintf(weight_string,"%5.0f", weight);

     	CanvasTextSet(&g_sLoadCellValue, weight_string);
        WidgetPaint((tWidget *)&g_sLoadCellValue);

    	TouchScreenCallbackSet(WidgetPointerMessage);

    	Wait(150);

    	WidgetMessageQueueProcess();
    }

}

// Handles presses of the Return To Main Button
void
OnReturnToMainButton(tWidget *pWidget)
{
    //kill the sampling widget
    main_menu=TRUE;

    Wait (100); // wait for last message to be updated on information screen

	// Set the title of this panel.
	CanvasTextSet(&g_sTitle, "   Main Menu   ");
    WidgetPaint((tWidget *)&g_sTitle);

    TurnOffReturnToMain();

	// Remove the previous screen Menu
    WidgetRemove((tWidget *)&g_psCalibrationPanel);
    WidgetRemove((tWidget *)&g_sCalibrationStep2);
    WidgetRemove((tWidget *)&g_psTestPanel);
    WidgetRemove((tWidget *)&g_StartTestButton);
    WidgetRemove((tWidget *)&g_psInfoPanel);

    // Add and draw the Main Menu panel
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_psMainPanel);
    WidgetPaint((tWidget *)&g_psMainPanel);
}

void TurnOnReturnToMain(void)
{
	PushButtonCallbackSet(&g_sReturnToMain, OnReturnToMainButton);
	PushButtonOutlineOn(&g_sReturnToMain);
    PushButtonTextOn(&g_sReturnToMain);
    PushButtonFillOff(&g_sReturnToMain);
    WidgetPaint((tWidget *)&g_sReturnToMain);
    main_menu=FALSE;
}
void TurnOnStartTest(void)
{
	PushButtonCallbackSet(&g_StartTestButton, OnStartTestButton);
	PushButtonOutlineOn(&g_StartTestButton);
    PushButtonTextOn(&g_StartTestButton);
    PushButtonFillColorSet(&g_StartTestButton, ClrWhite);
    PushButtonFillOn(&g_StartTestButton);
    WidgetPaint((tWidget *)&g_StartTestButton);
}
void TurnOffReturnToMain(void)
{
	PushButtonCallbackSet(&g_sReturnToMain, 0);
	PushButtonOutlineOff(&g_sReturnToMain);
    PushButtonTextOff(&g_sReturnToMain);
    PushButtonFillOn(&g_sReturnToMain);
    WidgetPaint((tWidget *)&g_sReturnToMain);
}

void TurnOffStartTest(void)
{
	PushButtonCallbackSet(&g_StartTestButton, 0);
	PushButtonOutlineOff(&g_StartTestButton);
    PushButtonTextOff(&g_StartTestButton);
    PushButtonFillColorSet(&g_StartTestButton, ClrBlack);
    PushButtonFillOn(&g_StartTestButton);
    WidgetPaint((tWidget *)&g_StartTestButton);
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

	//add exception handling if value is out of expected range
	CanvasTextSet(&g_sCalStep1Result, "...Success...");
	CanvasTextOpaqueOn(&g_sCalStep1Result);
	WidgetPaint((tWidget *)&g_sCalStep1Result);

    // Change Text of Button
	PushButtonTextSet(&g_CalStep1Button, "Done");
	PushButtonCallbackSet(&g_CalStep1Button, 0);
    WidgetPaint((tWidget *)&g_CalStep1Button);

    //Add Cal Step 2 to Widget Tree
	WidgetAdd((tWidget *)&g_psCalibrationPanel, (tWidget *)&g_sCalibrationStep2);
    WidgetPaint((tWidget *)&g_sCalibrationStep2);
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

	//add exception handling if value is out of expected range
	CanvasTextSet(&g_sCalStep2Result, "...Success...");
	CanvasTextOpaqueOn(&g_sCalStep2Result);
    WidgetPaint((tWidget *)&g_sCalStep2Result);

    // Change Text of Button
	PushButtonTextSet(&g_CalStep2Button, "Done");
	PushButtonCallbackSet(&g_CalStep2Button, 0);
    WidgetPaint((tWidget *)&g_CalStep2Button);

}


void OnStartTestButton (tWidget *pWidget)
{
	// Set motion parameters (could be configurable in GUI later)
	float total_move_distance=5;  //mm
	float travel_speed=2;   //mm/s

	uint32_t total_move_steps = mm_to_steps(total_move_distance);

	float peak_loadcell_reading=0.0;
	char result[40];

	// Disable Test and return to main buttons during test
	TurnOffStartTest();
	//WidgetRemove((tWidget *)&g_StartTestButton);
	TurnOffReturnToMain();
	WidgetMessageQueueProcess();

	//Start Driving Down with measurements

	CanvasTextSet(&g_TestingResults, "...Moving Down...");
	WidgetPaint((tWidget *)&g_TestingResults);
	WidgetMessageQueueProcess();

	move_motor(travel_speed, total_move_steps, DOWN);

	while(steps_to_move>0){

		// Make Measurement and write to peak if highest
		LoadCell = hx711_average(1); // take 1 sample for fast timing
		weight = scaled_reading(LoadCell);
		if (weight>peak_loadcell_reading) peak_loadcell_reading=weight;

		//Update Screen?

	}

	CanvasTextSet(&g_TestingResults, "Finished..Moving Up-Please Wait...");
	WidgetPaint((tWidget *)&g_TestingResults);
	WidgetMessageQueueProcess();

	travel_speed = 5;
	move_motor(travel_speed, total_move_steps, UP);

	while (steps_to_move>0){}  //wait for motor to stop moving

	// Display test result - add logic to determine pass/fail
	if (peak_loadcell_reading*0.00220462>3.0){
		sprintf(result,"Cap FAILED - Peak force measured %2.3f pounds", peak_loadcell_reading*0.00220462);
	}
	else if (peak_loadcell_reading*0.00220462<=3.0){
		sprintf(result,"Cap PASSED - Peak force measured %2.3f pounds", peak_loadcell_reading*0.00220462);
	}

	CanvasTextSet(&g_TestingResults, result);

    WidgetPaint((tWidget *)&g_TestingResults);
	WidgetMessageQueueProcess();

	// Write test result to file on USB?

    //Enable Test button with new text and return to main
	TurnOnStartTest();
	TurnOnReturnToMain();
	PushButtonTextSet(&g_StartTestButton, "Test Another");
	WidgetPaint((tWidget *)&g_StartTestButton);
	WidgetMessageQueueProcess();
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
	CanvasTextSet(&g_sTitle, "   Main Menu   ");

	// Issue the initial paint request to the widgets.
	WidgetPaint(WIDGET_ROOT);

	// Loop forever handling widget messages.
	while (1)
	{
		// Process any messages in the widget message queue.
		WidgetMessageQueueProcess();
	}
}

