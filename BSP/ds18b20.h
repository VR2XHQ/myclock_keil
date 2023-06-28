/*
 * ds18b20.h
 *
 *  Created on: Sep 18, 2021
 *      Author: Ben Yuen
 */

#ifndef DS18B20_H_
#define DS18B20_H_

#include "main.h"

/* Set return code */
#define DS18B20_NO_ERR 0
#define DS18B20_ERR_NO_PRESENCE -1
#define DS18B20_ERR_NO_DEVICES -2
#define DS18B20_ERR_CRC_VERIFY_FAILED -3
#define DS18B20_ERR_WAIT_TIMEOUT -4
#define DS18B20_SUCCESSED(x) ((x) == 0)  //((x) >= 0)

#define do_CRC	1  //Set 1 if the DS18B20 is with normal CRC function when read ram

typedef struct _DS18B20_RomCode_t
{
	union{
		struct{
			uint64_t familyCode :8;  //Family ID
			uint64_t serialNumber :48;  //Serial number;
			uint64_t crcCode :8;  //CRC code
		};
		uint64_t romCode;
	};
} DS18B20_RomCode_t;

typedef enum _DS18B20_Resolution_t
{
	resolution_9Bit		= 0x1f,  //9bits convert time 93.75ms
	resolution_10Bit 	= 0x3f,  //10bits convert time 187.5ms
	resolution_11Bit	= 0x5f,  //11bits convert time 375ms
	resolution_12Bit	= 0x7f,  //12bits convert time 750ms
}DS18B20_Resolution_t;

typedef struct _DS18B20_ConfigInfo
{
	int8_t alarmTH;  //Alarm upper limit
	int8_t alarmTL;  //Alarm lower limit
	uint8_t resolution;  //Resolution for temperature conversion
}DS18B20_ConfigInfo_t;

extern int32_t Temper_Var;  //Temperature variable
extern float temper_var;  //Temperature variable
extern DS18B20_RomCode_t RomID[10];  //Maximum 5 ROM ID stored in array, [0] for single ROM attached
extern DS18B20_RomCode_t AlarmID[10];

void DS18B20_check_us_timming(uint8_t _set);
uint8_t DS18B20_Reset(void);
int8_t DS18B20_SearchRom(DS18B20_RomCode_t* pRomList, uint8_t* numberOfRom);
int8_t DS18B20_SearchAlarm(DS18B20_RomCode_t* pRomList, uint8_t* numberOfRom);
int8_t DS18B20_ConvertTemperatureAll(void);  //Covert temperature for all attached DS18B20
int8_t DS18B20_ReadTemperature(DS18B20_RomCode_t* pRomCode, float* pTemperature);  //Read temperature for specific ROM
int8_t DS18B20_ReadTemper(DS18B20_RomCode_t* pRomCode, int32_t* pTemperature);  //Another method of temperature conversion
int8_t DS18b20_single_temper(int32_t* pTemperature);  //Read temperature for singl
int8_t DS18B20_SetConfig(const DS18B20_RomCode_t* pRom, const DS18B20_ConfigInfo_t* configInfo);
int8_t DS18B20_GetConfig(const DS18B20_RomCode_t* pRom, DS18B20_ConfigInfo_t* configInfo);
int8_t DS18B20_Read_romcode(DS18B20_RomCode_t* pRom);  //Get single ROM serial number
uint8_t DS18B20_if_parasite(void);  //Check if parasite power presence

#endif /* DS18B20_H_ */
