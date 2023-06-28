/*
 * SD3078_rtc_task.h
 *
 *  Created on: Nov 13, 2021
 *  Last edited on: Nov 17, 2022
 *      Author: Ben Yuen
 */

#ifndef SD3078_RTC_TASK_H_
#define SD3078_RTC_TASK_H_

//#include <stdlib.h>
//#include <stdio.h>
//#include "GUI.h"
//#include "Lcd_Driver.h"
#include "SD3078_rtc.h"
#include "ds18b20_task.h"

#define SW_Method	0  //Count base: 0=10ms, 1=Second,  Use system 1ms interrupt to generate Tenmillisecond and Second

/* define structure for menu access */
typedef struct
{
	uint8_t LCD;  //Display on/off status
	uint8_t UTC;  //0=local 1=UTC
	uint8_t AMPM;	//0=24hours 1=AM/PM
	uint8_t AL_mode;  //Alarm mode: 0=time 1=other
	uint8_t IM;  //Alarm INT mode
	uint8_t MODE;  //Sub-menu access control, 0=root
	uint8_t STATUS;  //Flow control: 0=default, 1=time editing, 9=other
	uint8_t wa;  //Value of weekday alarm setting
	uint8_t ea;  //Value of enable alarm setting
	uint8_t Temper_Source;  //0=DS18B20, 1=RTC

} Menu_Def;

extern Menu_Def m;  //For data exchange between function call
extern uint32_t TenMillisecond;  //Variable of 10ms count
extern uint32_t Second;  //Variable for 1s value

void Beep(uint8_t _count);
void Draw_title(uint8_t _set);
void Draw_menu(uint8_t _set);
void Clear_middle_display(uint8_t _set);
void Confirm_highlight(void);
void Reset_menu(void);
void MyClock_Init(void);
void Display_on_off(void);
void Toggle_AMPM(void);
void Alarm_show(void);
void Alarm_stop(void);
void Alarm_handle(void);
void Show_RTC_info(void);
void Show_reg_info(void);
void Toggle_TS(void);
void Show_temperature(uint8_t _set);
void Show_digit(uint8_t _hour);
void Show_time(uint8_t _status, uint8_t _ps);
void Time_setting(uint8_t _mode);
void Set_repeat_alarm(void);
void Weekday_setting(void);
void Count_down(void);
void Stop_watch(void);
void Freq_out(void);
void TnB_adjust_conf(void);

#endif /* SD3078_RTC_TASK_H_ */
