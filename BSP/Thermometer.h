/*
 * Thermometer.h
 *
 *  Created on: Oct 10, 2021
 *  Last edited on:
 *      Author: Ben Yuen
 */

#ifndef THERMOMETER_H_
#define THERMOMETER_H_

#include "stm32f1xx_hal.h"

typedef struct temperature_t
{
	uint8_t period;  //Actually no use
	uint16_t temper;
}TemperatureList;

#define sec	1
#define min	2
#define update_time	min  //Select the period for update

void DrawThermometer(uint16_t dx);  //Initialize the thermometer
void UpdateThermometer(uint16_t dx);  //Update the thermometer and temperature value
void DrawGridlines(void);  //Draw the outline for temperature curve
void DrawTemperatureCurve(uint8_t on);  //Draw the temperature curve against time period, on=0 for restart, on=1 for normal run

#endif /* THERMOMETER_H_ */
