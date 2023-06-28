/*
 * SD3078_rtc.h
 *
 *  Created on: Jul 24, 2021
 *  Last edited on: Nov 17, 2022
 *      Author: Ben Yuen
 */

#ifndef SD3078_RTC_H_
#define SD3078_RTC_H_

#include "i2c.h"
#include "tim.h"

enum Freq{F_0Hz, F32KHz, F4096Hz, F1024Hz, F64Hz, F32Hz, F16Hz, F8Hz, \
					F4Hz, F2Hz, F1Hz, F1_2Hz, F1_4Hz, F1_8Hz, F1_16Hz, F_1s};

enum clk_Souce{S_4096Hz, S_1024Hz, S_1s, S_1min};

enum TnB_Alarm{ALARM_DISABLE, TL_EN=0x24, TH_EN=0x48, BL_EN=0x61, BH_EN=0x82};

enum TnB_charge{DIS_charge=0x03, EN_charge_10k=0x80, EN_charge_5k=0x81, En_charge_2k=0x82};

/* define structure for time information */
typedef	struct
{
	uint8_t	second;
	uint8_t	minute;
	uint8_t	hour;
	uint8_t	week;
	uint8_t	day;
	uint8_t	month;
	uint8_t	year;
} Time_Def;

/* define structure for count down parameter */
typedef	struct
{
	enum clk_Souce d_clk;	// count down clock frequency
	uint8_t		IM;			// IM=1, INT go low 250ms periodically
	uint8_t		ms;			// set value for millisecond
	uint8_t		ss;			// set value for second
	uint8_t		mm;			// set value for minute
	uint8_t		hh;			// set value for hour
	uint32_t	init_val;	// initial count down value
	uint32_t	ref_val;	// initial reference for each count down process
	uint32_t	dis_val;	// display count down value in process
} CountDown_Def;

/* define structure for temperature and battery */
typedef struct
{
	int8_t temper;	//0x16
	uint8_t batiic;	//0x17
	uint8_t charge;	//0x18
	uint8_t ctr4;	//0x19
	uint8_t bat8;	//0x1A
	uint8_t bat7;	//0x1B
	int8_t low;		//0x1C
	int8_t high;
	int8_t lowest;	//0x1E
	int8_t highest;
} Temper_Bat_Def;

/******************** Device Address *********************/
#define		RTC_Address     0x64
#define		Real_Time		0x00	// Time reading address
#define		Alarm_Time		0x07	// Alarm reading address

/******************** Control Register *******************/
#define		CTR1            0x0F	// WRTC3 & WRTC2, status flag
#define		CTR2            0x10	// WRTC1, set mode & enable INTDE/INTAE/INTFE
#define		CTR3            0x11	// ARST, set frequency
#define		CTR4			0x19	// Temperature and battery alarm
#define		WA				0x0A	// weekday alarm
#define		EA				0x0E	// enable alarm

/***************** Timer Counter Register ****************/
#define		Timer_Counter1	0x13
#define		Timer_Counter2	0x14
#define		Timer_Counter3	0x15

/******************** Battery Register *******************/
#define		Chg_MG          0x18	// set charge condition

/****** Temperature Register and enable setting **********/
#define		TnB_address		0x16	// TnB start address
#define		Temper_ALM		0x1C	// Low & High temperature alarm
#define		Temper_REC		0X1E	// Low & High temperature record

/*********************** ID Register *********************/
#define		ID_Address		0x72		// ID beginning address

/************ 0EH enable alarm with different bit set ****************/
#define		sec_ALM			0x01	// for second
#define		min_ALM			0x02	// for minute
#define		hor_ALM			0x04	// for hour
#define		wek_ALM			0x08	// for week, valid only if day not set
#define		day_ALM			0x10	// for day
#define		mon_ALM			0x20	// for month
#define		yar_ALM			0x40	// for year

/********************** 10H register bit setting *********************/
#define		INST1			0x20	// INT type set by bit5
#define		INST0			0x10	// INT type set by bit4
#define		INTDE			0x04	// count down enable by bit2
#define		INTAE			0x02	// alarm enable by bit1
#define		INTFE			0x01	// freq_out enable by bit0

/**************** User RAM memory address 2CH - 71H ******************/
#define		ALM_time		0x2C	// 2CH-2FH save a copy of alarm time value
#define		AM_PM			0x30	// save status of time display method, 0=24hours, 1=AM/PM
#define		IM_flag			0x31	// save a copy of IM status
#define		CD_value		0x32	// 32H - 34H save a copy of count down value
#define		Freq_value		0x35	// save a copy of frequency out value
#define		WA_time			0x36	// 36H-37H keep weekly alarm hh:mm for reuse
#define		TS				0x38	// save option of temperature source, 0=DS18B20, 1=RTC

/* structure declaration */
extern Time_Def Time;  // time structure
extern Temper_Bat_Def TnB;  // temperature and battery structure
extern CountDown_Def cd;

/* function declaration */
void ReadByte(uint8_t _address, uint8_t _size, uint8_t *_Buff);  // read data from specific address
void WriteByte(uint8_t _address, uint8_t _size, uint8_t *_Buff);  // write data to specific address
void ClearAlarm(uint8_t _hanlde, uint8_t _im_flag);  // clear alarm flag
void ReadTime(Time_Def *time);  // read time from real time register
void WriteTime(Time_Def _time);  // set time to real time register
void SetAlarm(uint8_t _ea_flag, uint8_t wa_flag, uint8_t _im_flag, Time_Def _time);  // set alarm parameter
void SetCountDown(uint8_t _en, CountDown_Def *CountDown_Init);  // set count down parameter
void SetFreq(uint8_t _enable, enum Freq F_Out);  // set frequency output from INT (pin 7)
void TnB_GetValue(Temper_Bat_Def *TnB);  //Get temperature and battery configuration
void TnB_SetValue(Temper_Bat_Def *_Alarm_THL);  //Save temperature and battery configuration
void Reset_THL_Record(void);  //Reset the temperature record

#endif /* SD3078_RTC_H_ */
