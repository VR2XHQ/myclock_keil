/*
 * SD3078_rtc.c
 *
 *  Created on: Jul 24, 2021
 *  Last edited on: Nov 17, 2022
 *      Author: Ben Yuen
 */

#include "SD3078_rtc.h"

uint8_t TimeBuff[7];  // array convert time data before write action

/* structure declaration */
Time_Def Time;  // time structure
Temper_Bat_Def TnB;  // temperature and battery structure
CountDown_Def cd;

inline static void WriteRTC_Enable(void)
{
	uint8_t buf;  // using &buf as address pointer
	buf = 0x80;	// set WRTC1=1
	HAL_I2C_Mem_Write(&hi2c2, RTC_Address, CTR2, I2C_MEMADD_SIZE_8BIT, &buf, 0x01, 0x10);
	buf = 0xFF;	// set WRTC2=1,WRTC3=1; optional 0x84
	HAL_I2C_Mem_Write(&hi2c2, RTC_Address, CTR1, I2C_MEMADD_SIZE_8BIT, &buf, 0x01, 0x10);
//    /* other method */
//	uint8_t data[2];
//	data[0] = CTR2;
//	data[1] = 0x80;	// set WRTC1=1
//	HAL_I2C_Master_Transmit(&hi2c2, RTC_Address, data, 0x02, 0x10);
//	data[0] = CTR1;
//	data[1] = 0xFF;	// set WRTC2=1,WRTC3=1; optional 0x84
//	HAL_I2C_Master_Transmit(&hi2c2, RTC_Address, data, 0x02, 0x10);
}

inline static void WriteRTC_Disable(void)
{
	uint8_t buf[2];  // using array name as address pointer
	buf[0] = 0x7B;	// set WRTC2=0,WRTC3=0; optional 0x00
	buf[1] = 0x00;	// set WRTC1=0
	HAL_I2C_Mem_Write(&hi2c2, RTC_Address, CTR1, I2C_MEMADD_SIZE_8BIT, buf, 0x02, 0x10);
//    /* other method */
//	uint8_t data[3];
//	data[0] = CTR1;
//	data[1] = 0x7B;	// set WRTC2=0,WRTC3=0; optional 0x00
//	data[2] = 0x00;	// set WRTC1=0
//	HAL_I2C_Master_Transmit(&hi2c2, RTC_Address, data, 0x03, 0x10);
}

void ReadByte(uint8_t _address, uint8_t _size, uint8_t *_Buff)
{
	uint8_t buf[_size];
	HAL_I2C_Mem_Read(&hi2c2, RTC_Address, _address, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&buf, _size, 0x10);
	uint8_t i;
	for(i=0;i<=_size-1;i++)
	_Buff[i] = buf[i];
}

void WriteByte(uint8_t _address, uint8_t _size, uint8_t *_Buff)
{
	WriteRTC_Enable();
	HAL_I2C_Mem_Write(&hi2c2, RTC_Address, _address, I2C_MEMADD_SIZE_8BIT, _Buff, _size, 0x10);
	WriteRTC_Disable();
}

void ReadTime(Time_Def *time)
{
	Time_Def _time;  // temporary variable for time data conversion
	HAL_I2C_Master_Receive(&hi2c2, RTC_Address, (uint8_t *)&_time, sizeof(Time_Def), 0x10);
	_time.hour &= 0x7F;  // 24 hour mode need to clear bit7 for correct value
	/* convert time data from BCD to binary after read */
	time->second = (_time.second>>4)*10	+	(_time.second&0xf);
	time->minute = (_time.minute>>4)*10	+	(_time.minute&0xf);
	time->hour	 = (_time.hour>>4)*10	+	(_time.hour&0xf);
	time->week	 = (_time.week>>4)*10	+	(_time.week&0xf);
	time->day	 = (_time.day>>4)*10	+	(_time.day&0xf);
	time->month	 = (_time.month>>4)*10	+	(_time.month&0xf);
	time->year	 = (_time.year>>4)*10	+	(_time.year&0xf);
}

void WriteTime(Time_Def _time)
{
	WriteRTC_Enable();
	/* convert time data from binary to BCD before write */
	TimeBuff[0] = (_time.second/10)<<4	|   (_time.second%10);
	TimeBuff[1] = (_time.minute/10)<<4	|	(_time.minute%10);
	TimeBuff[2] = ((_time.hour/10)<<4	|	(_time.hour%10)) | 0x80;  // set bit7 to 1 for 24 hour mode
	TimeBuff[3] = (_time.week/10)<<4	|	(_time.week%10);
	TimeBuff[4] = (_time.day/10)<<4		|	(_time.day%10);
	TimeBuff[5] = (_time.month/10)<<4	|	(_time.month%10);
	TimeBuff[6] = (_time.year/10)<<4	|	(_time.year%10);
	HAL_I2C_Mem_Write(&hi2c2, RTC_Address, Real_Time, I2C_MEMADD_SIZE_8BIT, TimeBuff, sizeof(Time_Def), 0x10);
	WriteRTC_Disable();
}

void ClearAlarm(uint8_t _handle, uint8_t _im_flag)
{
	uint8_t buf[3];
	uint8_t ctr3;
	ReadByte(CTR3,0x01,&ctr3);
	/* Clear alarm option */
	if(_handle==0){
		WriteRTC_Enable();
		buf[0] = CTR2;
		buf[1] = 0x80|_im_flag<<6|INST0|INTAE;	//Restore to time alarm
		buf[2] = 0x80|ctr3;					//Set ARST to 1, keep original ctr3 status
		HAL_I2C_Master_Transmit(&hi2c2, RTC_Address, buf, 0x03, 0x10);
		WriteRTC_Disable();
		uint8_t dummy = 0;
		ReadByte(CTR1,0x01,&dummy);		//Clear INTAF flag by read CTR1 while ARST=1
	}
	if(_handle==1){
		WriteRTC_Enable();
		buf[0] = CTR2;
		buf[1] = 0x80|_im_flag<<6;			//Clear INST0/INST1/INTDE/INTAE/INTFE for alarm of temperature and battery
		HAL_I2C_Master_Transmit(&hi2c2, RTC_Address, buf, 0x02, 0x10);
		WriteRTC_Disable();
	}
}

void SetAlarm(uint8_t _ea_flag, uint8_t _wa_flag, uint8_t _im_flag, Time_Def _time)  // set alarm when and how
{
	WriteRTC_Enable();
	/* convert time data from binary to BCD before write */
	TimeBuff[0] = (_time.second/10)<<4	|   (_time.second%10);
	TimeBuff[1] = (_time.minute/10)<<4	|	(_time.minute%10);
	TimeBuff[2] = (_time.hour/10)<<4	|	(_time.hour%10);
	TimeBuff[3] = _wa_flag;  // weekday alarm set by caller
	TimeBuff[4] = (_time.day/10)<<4		|	(_time.day%10);
	TimeBuff[5] = (_time.month/10)<<4	|	(_time.month%10);
	TimeBuff[6] = (_time.year/10)<<4	|	(_time.year%10);
	TimeBuff[2] |= 0x80;  // set bit7 to 1 for 24 hour mode
	HAL_I2C_Mem_Write(&hi2c2, RTC_Address, Alarm_Time, I2C_MEMADD_SIZE_8BIT, TimeBuff, sizeof(Time_Def), 0x10);

	uint8_t buf[4];
	/* save IM status at user memory */
	buf[0] = _im_flag;
	HAL_I2C_Mem_Write(&hi2c2, RTC_Address, IM_flag, I2C_MEMADD_SIZE_8BIT, buf, 0x01, 0x10);
	/* Set Alarm type */
	buf[0] = _ea_flag;	// alarm enable flag set by caller
	buf[1] = 0xFF;	 // CTR1; optional 0x84
	buf[2] = 0x80|_im_flag<<6|INST0|INTAE;	// CTR2 alarm enable
	HAL_I2C_Mem_Write(&hi2c2, RTC_Address, EA, I2C_MEMADD_SIZE_8BIT, buf, 0x03, 0x10);
	/* save date&time alarm value at user memory for reference */
	buf[0] = _time.minute;	// 2CH
	buf[1] = _time.hour;	// 2DH
	buf[2] = _time.day;		// 2EH
	buf[3] = _time.month;	// 2FH
	HAL_I2C_Mem_Write(&hi2c2, RTC_Address, ALM_time, I2C_MEMADD_SIZE_8BIT, buf, 0x04, 0x10);
	/* If call for setting of weekly alarm, save hh:mm value for recall */
	if(_ea_flag==0x0E){
		HAL_I2C_Mem_Write(&hi2c2, RTC_Address, WA_time, I2C_MEMADD_SIZE_8BIT, buf, 0x02, 0x10);
	}
	WriteRTC_Disable();
}

void SetCountDown(uint8_t _en, CountDown_Def *CountDown_Init)
{
	uint8_t buf[4];
	if(_en==TRUE)
	{
	WriteRTC_Enable();
	/* Configure count down parameter */
	buf[0] = CTR2;
	buf[1] = (CountDown_Init->IM<<6)|0x80|INST1|INST0|INTDE;	// CTR2; IM|0xB4
	buf[2] = CountDown_Init->d_clk<<4;  // set TDS1 and TDS0, 0x20 for 1 second
	HAL_I2C_Master_Transmit(&hi2c2, RTC_Address, buf, 0x03, 0x10);
	/* Set initial count down value */
	buf[0] = Timer_Counter1;  // covert uint32_t count down value to 3 8bit register
	buf[1] = (CountDown_Init->init_val & 0x0000FF);			// 13H
	buf[2] = (CountDown_Init->init_val & 0x00FF00) >> 8;	// 14H
	buf[3] = (CountDown_Init->init_val & 0xFF0000) >> 16;	// 15H
	HAL_I2C_Master_Transmit(&hi2c2, RTC_Address, buf, 0x04, 0x10);
	WriteRTC_Disable();
	}
	else  // disable INTDE
	{
		*buf = 0xC0|INST1|INST0;;
		WriteByte(CTR2,0x01,buf);
	}
}

void SetFreq(uint8_t _enable, enum Freq F_Out)  // on/off frequency out with preset value
{
	uint8_t buf[3];
	WriteRTC_Enable();
	if(_enable==1)
	{
		buf[0] = CTR2;
		buf[1] = 0x80|INST1|INTFE;		 	// set WRTC1,INST1,INTFE=1; 0x21
		buf[2] = 0x80|F_Out;				// set frequency by main process
		HAL_I2C_Master_Transmit(&hi2c2, RTC_Address, buf, 0x03, 0x10);
	}
	else
	{
		buf[0] = CTR2;
		buf[1] = 0x80|0x40|INST0|INTAE;	// clear INTFE; restore to INTAE/IM=1
		buf[2] = 0x80|F_Out;				// set no frequency
		HAL_I2C_Master_Transmit(&hi2c2, RTC_Address, buf, 0x03, 0x10);
	}
	WriteRTC_Disable();
}

void TnB_GetValue(Temper_Bat_Def *TnB)
{
	Temper_Bat_Def _TnB;
	ReadByte(TnB_address,sizeof(Temper_Bat_Def),(uint8_t*)&_TnB);
	TnB->temper = _TnB.temper;
	TnB->batiic = _TnB.batiic;
	TnB->charge = _TnB.charge;
	TnB->ctr4 = _TnB.ctr4;
	TnB->bat8 = _TnB.bat8;
	TnB->bat7 = _TnB.bat7;
	TnB->low = _TnB.low;
	TnB->high = _TnB.high;
	TnB->lowest = _TnB.lowest;
	TnB->highest = _TnB.highest;
}

void TnB_SetValue(Temper_Bat_Def *_Alarm_THL)
{
	WriteByte(Chg_MG, 0x01, &_Alarm_THL->charge);  //Set battery charge type
	WriteByte(CTR4,0x01,&_Alarm_THL->ctr4);  //CTR4, function only when (INTS1,INTS0)=0
	int8_t buf[2];  //Set Low & High temperature alarm value
	buf[0] = _Alarm_THL->low;
	buf[1] = _Alarm_THL->high;
	WriteByte(Temper_ALM,0x02,(uint8_t*)buf);
}

void Reset_THL_Record(void)
{
	uint8_t buf[2];
	buf[0] = 0x07F;
	buf[1] = 0x80;
	WriteByte(Temper_REC,0x02,buf);
}
