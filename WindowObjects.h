/*
 * WindowObjects.h
 *
 *  Created on: Jun 18, 2017
 *      Author: slide
 */

#ifndef WINDOWOBJECTS_H_
#define WINDOWOBJECTS_H_

//Universal Objects
extern tCanvasWidget g_sTitle;
extern tPushButtonWidget g_sReturnToMain;



//Main Window Objects
extern tCanvasWidget g_psMainPanel;

extern tPushButtonWidget g_CalZButton;
extern tPushButtonWidget g_ScaleReadingButton;
extern tPushButtonWidget g_TestConfigButton;
extern tPushButtonWidget g_CalibrateScaleButton;
extern tPushButtonWidget g_TestScreenButton;

//Test Cap Window Objects
extern tCanvasWidget g_psTestPanel;

extern tCanvasWidget g_TestData;
extern tPushButtonWidget g_StartTestButton;
extern tCanvasWidget g_TestResult;

//Calibrate Scale Window Objects
extern tCanvasWidget g_psCalibrateScalePanel;

extern tCanvasWidget g_sCalibrationStep1;
extern tCanvasWidget g_sCalStep1Result;
extern tPushButtonWidget g_CalStep1Button;

extern tCanvasWidget g_sCalibrationStep2;
extern tCanvasWidget g_sCalStep2Result;
extern tPushButtonWidget g_CalStep2Button;

//Calibrate Z Window Objects
extern tCanvasWidget g_psCalZPanel;

extern tCanvasWidget g_sCalZStep1;
extern tCanvasWidget g_sCalZStep1Result;
extern tPushButtonWidget g_CalZStep1Button;

extern tCanvasWidget g_sCalZStep2;
extern tCanvasWidget g_sCalZStep2Result;
extern tPushButtonWidget g_CalZStep2Button;


//Scale Reading Window Objects
extern tCanvasWidget g_psScaleReadingPanel;

extern tCanvasWidget g_sLoadCellValue;


#endif /* WINDOWOBJECTS_H_ */
