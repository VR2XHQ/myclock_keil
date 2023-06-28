/*
 * ds18b20_task.h
 *
 *  Created on: Nov 3, 2021
 *      Author: Ben Yuen
 */

#ifndef DS18B20_TASK_H_
#define DS18B20_TASK_H_

#include <stdlib.h>
#include <stdio.h>
#include "ds18b20.h"
#include "GUI.h"
#include "Thermometer.h"

#define method  0  //Set 1 to calculate temperature by different method

void Front_page(void);
void ds18b20_adjust_conf(void);  //Set data to configuration register
void ds18b20_found_rom(uint8_t _count);  //Search ROM with total limit assigned
void ds18b20_found_alarm(uint8_t _count);  //Search alarm with total limit assigned
int8_t ds18b20_check(void);  //Check single device RomCode and its configuration
void ds18b20_single_temper(int8_t *_str);  //Get single device temperature
void ds18b20_list_temper(void);  //List temperature of DS18B20 found, maximum 8 devices
void ShowThermometer(void);  //Run the thermometer graphic
void DS18B20_function_set(void);


#endif /* DS18B20_TASK_H_ */
