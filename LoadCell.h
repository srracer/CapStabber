/*
 * LoadCell.h
 *
 *  Created on: Jun 17, 2017
 *      Author: slide
 */

#ifndef LOADCELL_H_
#define LOADCELL_H_



//Load Cell Settings

#define CAL_WEIGHT 71                // ground down carbide rod is 71g
#define CAL_MEASUREMENTS 25

//Declarations
// These variables are used to store the Load Cell measurement data
extern char weight_string[8];
extern int32_t LoadCell;
extern int32_t loadcell_cal_zero;
extern int32_t loadcell_cal_weight;
extern uint32_t cal_data_eeprom[2];  //unsigned for eeprom writing
extern uint32_t number_of_measurements;
extern float weight;

//Load Cell functions
void hx711_init (void);
int32_t hx711_getvalue(void);
int32_t hx711_average(int8_t samples);
void hx711_power_down(void);
void hx711_power_up(void);
float scaled_reading(int32_t);
float convert_to_pounds(float);

#endif /* LOADCELL_H_ */
