/*
 * SD3078_rtc_task.c
 *
 *  Created on: Nov 13, 2021
 *  Last edited on: Nov 17, 2022
 *      Author: Ben Yuen
 */

#include "SD3078_rtc_task.h"

Menu_Def m;  //For data exchange between function call
uint32_t T_10ms;  //Variable of 10ms count
uint32_t T_100ms;  //Variable of 100ms count
uint32_t T_1000ms;  //Variable for 1000ns count

static uint8_t ShowBuff[48];  //Buff for content display
static uint8_t x=4,y=28;  //Axis origin for display area
static uint8_t old_minute;  //For refresh hour/minute digit display
static uint32_t old_second;  //For count down refresh interval 
static uint16_t _time_color[1];  //For switch color between UTC and LOCAL time
static uint16_t _I_color[1];  //Text color of menu instruction

/*Text for function display */
const char *_week_text[7]={"SUN","MON","TUE","WED","THU","FRI","SAT"};  //Weekday display
const char *_freq_text[15]={"32KHz","4096Hz","1024Hz","64Hz","32Hz","16Hz","8Hz","4Hz","2Hz","1Hz","1/2Hz","1/4Hz","1/8Hz","1/16Hz","1s"};  //Freq_Out display
const char *_time_text[6]={"YR","MM","DD","hh","mm","WK"};  //Time set menu display

/* Text for menu display */
const char _Submenu_0[]={"  UTC   DS18B20   TIMER  MODE-->"};  //Root menu text
const char _Submenu_10[]={"Esc    F2   F3      "};  //Timer sub-menu text
const char _Submenu_1[]={"Esc         Set<Conf"};  //Configuration sub-menu text
const char _Submenu_2[]={"Esc   HrAL  Set<AL_1"};  //Single incident alarm sub-menu text
const char _Submenu_3[]={"Esc   Wset  Set<AL_2"};  //Weekly alarm sub-menu text
const char _Submenu_4[]={"Esc   More  Set<Time"};  //Current time set sub-menu text
const char _Submenu_41[]={"Esc   INFO  TS      "};
const char _Submenu_42[]={"Esc  SD3078 RTC info"};
const char _Submenu_5[]={"Esc   More  Freq -->"};
const char _Submenu_51[]={"Esc   F2    F3   F4 "};

/* For UTC display month/day correction while rolling to first day of month */
const uint8_t last_day_of_month[12]={31,28,31,30,31,30,31,31,30,31,30,31};

inline void Beep(uint8_t _count)
{
	for(uint8_t i=0;i<_count;i++){
		SP_ON();HAL_Delay(100);SP_OFF();HAL_Delay(50);
	}
}

inline void Draw_title(uint8_t _set)  //Show or clear title
{
	if(_set==TRUE){
		Gui_DrawFont_GBK24(0,0,C_GOLD,C_BLUE,(uint8_t*)"   MY CLOCK  ");
	}
	else{
		Lcd_Clear(0, 0, 159, 23, C_BLACK);  //Clear title display
	}
}

void Draw_menu(uint8_t _set)  //Refresh menu bar and show information above
{
	uint8_t buf[4];
	uint8_t ctr2,_im;
	*_I_color = C_CYAN;  //Set text color of menu instruction

	Clear_middle_display(0);
	switch(_set)
	{
	case 0:
		Lcd_Clear(0,111, 159, 127, C_BLACK);  //Clear menu bar
		Gui_DrawFont_GBK8(0,118,C_WHITE,C_BLUE,(uint8_t*)_Submenu_0);  //Show main menu function
		Gui_DrawFont_GBK8(x,y+74,*_I_color,C_BLACK,(uint8_t*)"<|> LCD on/off  <-- Alarm mode");  //Show instruction

		/* Read alarm configuration and check if activate */
		ReadByte(CTR2,0x01,&ctr2);
		_im = ctr2;  //Extract IM bit value
		if(ctr2==0x12 || ctr2==0x52){
			if(m.AL_mode==1){m.AL_mode=0;}  //Avoid set alarm under m.AL_mode=1 condition
			ReadByte(ALM_time,0x04,buf);  //Read the copy of alarm date&time stored at 2CH-2FH
			switch(m.ea)
			{
			case 0x02:
				sprintf((char*)ShowBuff,"IM=%d hourly alarm at mm 00",(_im&0x40)>>6);
				Gui_DrawFont_GBK8(x,y+64,C_WHITE,C_RED,(uint8_t*)"R");  //Hourly repeat
				break;
			case 0x36:
				sprintf((char*)ShowBuff,"IM=%d Time alarm %02d/%02d %02d:%02d",m.IM,*(buf+3),*(buf+2),*(buf+1),*(buf+0));
				Gui_DrawFont_GBK8(x,y+64,C_WHITE,C_RED,(uint8_t*)"S");  //Specific date&time
				break;
			case 0x0E:
				sprintf((char*)ShowBuff,"IM=%d weekly alarm at %02d:%02d",m.IM,*(buf+1),*(buf+0));
				Gui_DrawFont_GBK8(x,y+64,C_WHITE,C_RED,(uint8_t*)"W");  //Weekday repeat
				break;
			default:
				Gui_DrawFont_GBK8(x,y+64,C_RED,C_BLACK,(uint8_t*)"Unknown");  //In case wrong input
				break;
			}
		}
		else{
			if(ctr2==0x00 || ctr2==0x40){
			sprintf((char*)ShowBuff,"IM=%d Other alarm activated",m.IM);
			Gui_DrawFont_GBK8(x,y+64,C_WHITE,C_RED,(uint8_t*)"O");  //Other alarm
			}
		}
		Gui_DrawFont_GBK8(x+10,y+64,C_GREEN_YELLOW,C_BLACK,ShowBuff);  //Show text of alarm type and time set
		break;
	case 10:
		Gui_DrawFont_GBK16(0,112,C_LIME,C_BLUE,(uint8_t*)_Submenu_10);
		Gui_DrawFont_GBK8(x,y+64,*_I_color,C_BLACK,(uint8_t*)"F2 > Count down timer");
		Gui_DrawFont_GBK8(x,y+74,*_I_color,C_BLACK,(uint8_t*)"F3 > Stop watch");
		break;
	case 1:
		Gui_DrawFont_GBK16(0,112,C_WHITE,C_BLUE,(uint8_t*)_Submenu_1);
		if(m.AMPM==1){
			Gui_DrawFont_GBK16(40,112,C_WHITE,C_RED,(uint8_t*)"AM_PM");
		}
		else{
			Gui_DrawFont_GBK16(40,112,C_WHITE,C_RED,(uint8_t*)"24_HR");
		}
		Gui_DrawFont_GBK8(x,y+64,*_I_color,C_BLACK,(uint8_t*)"AM_PM / 24Hours selection");
		Gui_DrawFont_GBK8(x,y+74,*_I_color,C_BLACK,(uint8_t*)"Set > RTC Conf & Register info");
		break;
	case 2:
		Gui_DrawFont_GBK16(0,112,C_WHITE,C_BLUE,(uint8_t*)_Submenu_2);
		Gui_DrawFont_GBK8(x,y+64,*_I_color,C_BLACK,(uint8_t*)"HrAL > Hourly alarm");
		Gui_DrawFont_GBK8(x,y+74,*_I_color,C_BLACK,(uint8_t*)"Set one time alarm");
		break;
	case 3:
		Gui_DrawFont_GBK16(0,112,C_WHITE,C_BLUE,(uint8_t*)_Submenu_3);
		Gui_DrawFont_GBK8(x,y+64,*_I_color,C_BLACK,(uint8_t*)"Wset > Select weekday");
		Gui_DrawFont_GBK8(x,y+74,*_I_color,C_BLACK,(uint8_t*)"Set weekday alarm");
		break;
	case 4:
		Gui_DrawFont_GBK16(0,112,C_WHITE,C_BLUE,(uint8_t*)_Submenu_4);
		Gui_DrawFont_GBK8(x,y+64,*_I_color,C_BLACK,(uint8_t*)"More > Other function");
		Gui_DrawFont_GBK8(x,y+74,*_I_color,C_BLACK,(uint8_t*)"Set current time");
		break;
	case 5:
		Gui_DrawFont_GBK16(0,112,C_WHITE,C_BLUE,(uint8_t*)_Submenu_5);
		Gui_DrawFont_GBK8(x,y+64,*_I_color,C_BLACK,(uint8_t*)"More > Other function");
		Gui_DrawFont_GBK8(x,y+74,*_I_color,C_BLACK,(uint8_t*)"Freq generator");
		break;
	case 41:
		Gui_DrawFont_GBK16(0,112,C_WHITE,C_BLUE,(uint8_t*)_Submenu_41);
		Gui_DrawFont_GBK8(x,y+64,*_I_color,C_BLACK,(uint8_t*)"INFO > RTC details");
		Gui_DrawFont_GBK8(x,y+74,*_I_color,C_BLACK,(uint8_t*)"TS > Temperature source");
		break;
	case 42:
		Gui_DrawFont_GBK16(0,112,C_WHITE,C_BLUE,(uint8_t*)_Submenu_42);
		break;
	case 51:
		Gui_DrawFont_GBK16(0,112,C_WHITE,C_BLUE,(uint8_t*)_Submenu_51);
		Gui_DrawFont_GBK8(x,y+64,*_I_color,C_BLACK,(uint8_t*)"Reserve function call");
		Gui_DrawFont_GBK8(x,y+74,*_I_color,C_BLACK,(uint8_t*)"F2/F3/F4 > Not defined");
		break;
	default:
		Gui_DrawFont_GBK8(x,y+74,C_RED,C_BLACK,(uint8_t*)"Unknown");  //In case wrong input
		break;
	}
}

inline void Clear_middle_display(uint8_t _set)
{
	if(_set==TRUE){
		Lcd_Clear(0,24,159,111,C_BLACK);  //Clear area between title and menu bar
	}
	else{
		Lcd_Clear(0,92,159,111,C_BLACK);  //Clear information area above menu bar
	}
}

inline void Confirm_highlight(void)
{
	Gui_DrawFont_GBK16(88, 112, C_WHITE, C_RED, (uint8_t*)"*Set");
	HAL_Delay(200);  //Delay for visual effect
	Gui_DrawFont_GBK16(88, 112, C_WHITE, C_BLUE, (uint8_t*)" Set");
}

void Reset_menu(void)
{
	/* Reset to root menu */
	m.MODE = 0;
	m.STATUS = 0;
	Draw_title(enable);
	Draw_menu(0);
}

inline void MyClock_Init(void)  //Initialize screen display and value of certain variable
{
	Blink_LED2();Beep(2);
	Lcd_Init();LCDBK_ON();
	Lcd_Clear(0, 0, 159, 127, C_BLACK);  //Clear whole display
	m.LCD = 1;  //default display on
	m.UTC = 0;  //0=local, 1=UTC
	m.AL_mode = 0;  //0=time alarm, 1=other alarm
	ReadByte(WA,0x01,&m.wa);  //Copy 0AH weekday alarm to WA_flag
	ReadByte(EA,0x01,&m.ea);  //Copy 0EH enable alarm EA_flag
	/* IM: alarm INT output pin trigger mode, 0=keep pull down low, 1=250ms pull down pulse */
	ReadByte(IM_flag,0x01,&m.IM);  //Copy IM_flag status from RTC user memory
	m.IM=m.IM&0x01;  //Make sure value only 0 or 1
	ReadByte(AM_PM,0x01,&m.AMPM);  //Copy AM/PM status from RTC user memory: 0=24HR, 1=AM/PM
	m.AMPM=m.AMPM&0x01;  //Make sure value only 0 or 1
	ReadByte(TS,0x01,&m.Temper_Source);  //Copy TS option from RTC user memory: 0=DS18B20, 1=RTC
	m.Temper_Source=m.Temper_Source&0x01;  //Make sure value only 0 or 1
	Reset_menu();
	ClearAlarm(0,m.IM);  //Restore time alarm mode
	Show_time(9,0);  //Refresh time display once
	Show_temperature(m.Temper_Source);  //Refresh temperature display once
}

inline void Display_on_off(void)
{
	m.LCD ^= 0x01;  //Toggle screen on/off
	if(m.LCD==TRUE) {LCDBK_ON();LED1_ON();Beep(2);LED1_OFF();}
	else {LCDBK_OFF();LED1_ON();Beep(3);LED1_OFF();}
}

inline void Toggle_AMPM(void)
{
	m.AMPM^=0x01;
	WriteByte(AM_PM,0x01,&m.AMPM);  //Save the change of time display mode
	if(m.AMPM==1){
		Gui_DrawFont_GBK16(40,112,C_WHITE,C_RED,(uint8_t*)"AM_PM");
	}
	else{
		Gui_DrawFont_GBK16(40,112,C_WHITE,C_RED,(uint8_t*)"24_HR");
	}
	Show_time(9,0);  //Refresh time display once
	HAL_Delay(300);  //Delay to avoid further key press trigger again
}

inline void Alarm_show(void)
{
	Draw_title(disable);  //Clear title
	if(m.AL_mode==0){
		Gui_DrawFont_GBK24(0,0,C_WHITE,C_RED,(uint8_t*)"Time Alarm On");  //13 character
	}
	else{
		Gui_DrawFont_GBK24(0,0,C_YELLOW,C_RED,(uint8_t*)" Other Alarm ");  //13 character
	}

	/* In hourly alarm mode, no beep, just show alarm display few seconds */
	if(m.ea==0x02 && m.AL_mode==0){
		Beep(2);HAL_Delay(3000);
		Alarm_stop();  //Clear alarm automatically
	}
	else{
		Gui_DrawFont_GBK16(0,112,C_RED,C_BLACK,(uint8_t*)"   Press F1 or <|>   ");
		if(m.AL_mode==1){
			Beep(5);  //Short beep for other alarm
		}
		else{
			Beep(10);
		}
	}
}

inline void Alarm_stop(void)
{
	Gui_DrawFont_GBK16(0,112,C_GREEN,C_BLACK,(uint8_t*)"  *= Alarm Reset =* ");  //Show what happen
	HAL_Delay(1000);  //Show text for 1 second then execute
	ClearAlarm(0,m.IM);  //Restore time alarm configuration
	Reset_menu();
}

inline void Alarm_handle(void)
{
	Alarm_show();
	if(m.ea!=0x02 || m.AL_mode==1){  //Execute while in one time, weekday or other alarm
		do{
			HAL_Delay(50);  //Kill time
		}while(Read_F1()==0 && Read_PwrKey()==0);  //Wait for action
		Alarm_stop();
	}
}

inline void Show_RTC_info(void)
{
	uint8_t buf[8];
	Clear_middle_display(1);  //Clear display of date&time content

	/* Read chip unique ID */
	ReadByte(ID_Address,0x08,buf);
	sprintf((char*)ShowBuff,"PD: 20%02X/%02X/%02X  Machine# : %02X",buf[0],buf[1],buf[2],buf[3]);
	Gui_DrawFont_GBK8(x,y+0,C_CYAN,C_BLACK,ShowBuff);
	sprintf((char*)ShowBuff,"OS# : %02X%02X  SN# : %02X%02X",buf[4],buf[5],buf[6],buf[7]);
	Gui_DrawFont_GBK8(x,y+9,C_CYAN,C_BLACK,ShowBuff);

	TnB_GetValue(&TnB);  //Fetch data for display output

	/* Read battery voltage */
	uint16_t Vbat_val;
	Vbat_val = (TnB.bat8>>7)*255 + TnB.bat7;  //Value conversion
	sprintf((char*)ShowBuff,"Battery : %d.%d%dV  charge = %02xh",Vbat_val/100,Vbat_val%100/10,Vbat_val%10,TnB.charge);
	Gui_DrawFont_GBK8(x,y+20,C_ORANGE,C_BLACK,ShowBuff);

	/* Read temperature information */
	sprintf((char*)ShowBuff,"Current temperature %3d'C",TnB.temper);
	Gui_DrawFont_GBK8(x,y+30,C_LIGHT_BLUE,C_BLACK,ShowBuff);
	sprintf((char*)ShowBuff,"Low  ALm/Rec : %3d'C / %3d'C",TnB.low,TnB.lowest);
	Gui_DrawFont_GBK8(x,y+42,C_LIME,C_BLACK,ShowBuff);
	sprintf((char*)ShowBuff,"High ALm/Rec : %3d'C / %3d'C",TnB.high,TnB.highest);
	Gui_DrawFont_GBK8(x,y+64,C_RED,C_BLACK,ShowBuff);
	ReadByte(0x20,0x06,buf);  //Read lowest temperature record time
	sprintf((char*)ShowBuff,"at %02X/%02X/%02X %02X:%02X %s",buf[5],buf[4],buf[3],buf[1]&=0x7F,buf[0],_week_text[buf[2]]);
	Gui_DrawFont_GBK8(x+10,y+52,C_LIME,C_BLACK,ShowBuff);
	ReadByte(0x26,0x06,buf);  //Read highest temperature record time
	sprintf((char*)ShowBuff,"at %02X/%02X/%02X %02X:%02X %s",buf[5],buf[4],buf[3],buf[1]&=0x7F,buf[0],_week_text[buf[2]]);
	Gui_DrawFont_GBK8(x+10,y+74,C_RED,C_BLACK,ShowBuff);

	do{
		HAL_Delay(50);  //Kill time
	}while(Read_F1()==FALSE);  //Display information until press F1, then return to upper sub-menu
	Clear_middle_display(1);  //Clear the content
	Show_time(9,0);  //Refresh display
}

inline void Show_reg_info(void)
{
	Clear_middle_display(0);  //Clear content already display
	uint8_t buf[3];
  	ReadByte(CTR1,0x3,buf);  //Read RTC register value
	uint8_t _im = buf[1];  //Extract IM bit value
  	sprintf((char*)ShowBuff,"IM=%d  CTR1=%02X CTR2=%02X CTR3=%02X",(_im&0x40)>>6,buf[0],buf[1],buf[2]);
  	Gui_DrawFont_GBK8(x,y+64,C_WHITE,C_BLACK,ShowBuff);
	sprintf((char*)ShowBuff,"EA=%02X WA=%02X",m.ea,m.wa);
	Gui_DrawFont_GBK8(x,y+74,C_WHITE,C_BLACK,ShowBuff);
}

void Toggle_TS(void){
	m.Temper_Source^=0x01;
	WriteByte(TS,0x01,&m.Temper_Source);  //Save the option of temperature source
	if(m.Temper_Source==0){
		Gui_DrawFont_GBK16(96,112,C_WHITE,C_RED,(uint8_t*)"DS18B20");
	}
	else {
		Gui_DrawFont_GBK16(96,112,C_WHITE,C_RED,(uint8_t*)"RTC");
		Gui_DrawFont_GBK16(120,112,C_WHITE,C_BLUE,(uint8_t*)"    ");  //For effect
	}
	HAL_Delay(300);  //Delay to avoid further key press trigger again
}

inline void Show_temperature(uint8_t _set)
{
	/* Option of temperature source, 0=DS18B20, 1=RTC */
	if(_set==0){
//		DS18B20_ON();  //Use parasite power, comment out this
		int8_t buf[10];
		ds18b20_single_temper(buf);
		Gui_DrawFont_GBK8(x+115,y+6,*_time_color,C_BLACK,(uint8_t*)buf);
//		DS18B20_OFF();
	}
	else{
		TnB_GetValue(&TnB);
		sprintf((char*)ShowBuff,"%3d.0'C",TnB.temper);
		Gui_DrawFont_GBK8(x+115,y+6,*_time_color,C_BLACK,ShowBuff);
	}
}

inline void Show_digit(uint8_t _hour)
{
	uint8_t digit_display[6];  //Show utc time in digital font
	digit_display[0]=_hour/10;
	digit_display[1]=_hour%10;
	digit_display[2]=0x0B;  //:
	digit_display[3]=Time.minute/10;
	digit_display[4]=Time.minute%10;
	digit_display[5]=0x10;  //EOString
	Gui_DrawFont_Num32Str(x-4, y+24,*_time_color,C_BLACK,digit_display);
}

void Show_time(uint8_t _status, uint8_t _ps)
{
	/* Not update time while in time editing */
	if(_status!=1){
		ReadTime(&Time);
	}

	/* Covert local time to UTC and display relevant text&color */
	uint8_t _time_hour,_time_day,_time_month,_time_week,_time_year;
	_time_hour = Time.hour;
	_time_day = Time.day;
	_time_week = Time.week;
	_time_month = Time.month;
	_time_year = Time.year;
	if(m.UTC==1){  //Check utc status to switch accordingly
		if((_time_hour >= 8) && (_time_hour <= 23)){
			_time_hour = _time_hour-8;
		}
		else{  //During local time from hour of 0 - 7
			_time_hour = _time_hour+16;
			if(_time_week==0){
				_time_week=6;
			}
			else{
				_time_week = _time_week-1;
			}
			if(_time_day!=1){  //If not first day of month, do simple conversion.
				_time_day = _time_day-1;
			}
			else{  //If local time is first day of month, do below conversion.
				if(Time.year%4==0 && _time_month==3){  //Check if leap year.
					_time_day = 29;
				}
				else{  //Set UTC day to last day of previous month.
					if(_time_month==1){
						_time_day = last_day_of_month[11];
						_time_month = 12;
						_time_year = _time_year-1;
					}
					else{
						_time_day = last_day_of_month[_time_month-2];
						_time_month = _time_month-1;  //Show previous month.
					}
				}
			}
		}
		*_time_color = C_LIGHT_SKY_BLUE;  //Set UTC color
		Gui_DrawFont_GBK8(x+134,y+48,*_time_color,C_BLACK,(uint8_t*)"UTC");
	}
	else{
		*_time_color = C_LEMON_CHIFFON;  //Set LOCAL color
		Gui_DrawFont_GBK8(x+134,y+48,*_time_color,C_BLACK,(uint8_t*)"Loc");
	}

	/* Covert 24hours to AM/PM */
	if(m.AMPM==1){
		if(_time_hour>=12){
			if(_time_hour>12){
				_time_hour = _time_hour-12;
			}
			Gui_DrawFont_GBK8(x+137,y+40,C_WHITE,C_RED,(uint8_t*)"PM");
		}
		else{
			if(_time_hour==0){  //UTC 0 hour = 12AM
				_time_hour=12;
			}
			Gui_DrawFont_GBK8(x+137,y+40,C_WHITE,C_RED,(uint8_t*)"AM");
		}
	}
	else{
		Lcd_Clear(x+134,y+40,x+148,y+47,C_BLACK);  //Clear AM/PM text when in 24HR mode
	}

	/* Show date, weekday and second */
	sprintf((char*)ShowBuff,"20%02d/%02d/%02d",_time_year,_time_month,_time_day);
	Gui_DrawFont_GBK16(x,y,*_time_color,C_BLACK,ShowBuff);
	//HAL_Delay(50);  //Delay for visual effect
	Gui_DrawFont_GBK8(x+90,y+6,*_time_color,C_BLACK,(uint8_t*)_week_text[_time_week]);
	//HAL_Delay(50);  //Delay for visual effect
	sprintf((char*)ShowBuff,"%02d",Time.second);
	Gui_DrawFont_GBK16(x+134,y+24,*_time_color,C_BLACK,ShowBuff);

	/* Blink the relevant display position during time editing */
	if(_status==1){
		switch(_ps)
		{
		case 1:
			Gui_DrawFont_GBK16(x+16,y,C_WHITE,C_BLACK,(uint8_t*)"  ");  //year
			break;
		case 2:
			Gui_DrawFont_GBK16(x+16+(8*3),y,C_WHITE,C_BLACK,(uint8_t*)"  ");  //month
			break;
		case 3:
			Gui_DrawFont_GBK16(x+16+(8*6),y,C_WHITE,C_BLACK,(uint8_t*)"  ");  //day
			break;
		case 4:
			Gui_DrawFont_GBK16(x+22,y+24,C_RED,C_BLACK,(uint8_t*)"*");  //hour
			HAL_Delay(50);  //Delay for visual effect
			break;
		case 5:
			Gui_DrawFont_GBK16(x+100,y+24,C_RED,C_BLACK,(uint8_t*)"*");  //minute
			break;
		case 6:
			Gui_DrawFont_GBK8(x+90,y+6,C_WHITE,C_BLACK,(uint8_t*)"   ");  //weekday
			break;
		default:
			break;
		}
	}

	/* Update time digit and temperature every 1 minute while in root screen*/
	if(m.MODE==0 && _status==0){
		if(old_minute != Time.minute){
			Blink_LED1();
			Show_digit(_time_hour);
			Show_temperature(m.Temper_Source);
			old_minute = Time.minute;
		}
	}

	/* Redraw once */
	if(_status==1){  //Update the display once, no RTC read
		Show_digit(_time_hour);
	}
	if(_status==9){  //Update the display once when exit from function call
		Show_digit(_time_hour);
		Show_temperature(m.Temper_Source);
	}
}

void Time_setting(uint8_t _mode)
{
	uint8_t save_exit=1;  //Set 1 to enter loop
	uint8_t utc_flag=0;  //UTC status flag for time editing
	uint8_t _ps,Init_ps,_pe;  //Pointer for action of adjustment
	m.STATUS=1;  //Set flag for flow control

	/* Pointer start/end value based on menu mode */
	if(_mode==2){_ps=Init_ps=2;_pe=6;}
	if(_mode==3){_ps=Init_ps=4;_pe=6;}
	if(_mode==4){_ps=Init_ps=1;_pe=7;}
	Gui_DrawFont_GBK16(96,112,C_WHITE,C_BLUE,(uint8_t*)"   ");  //Clear text of "Set"
	Gui_DrawFont_GBK16(96,112,C_WHITE,C_RED,(uint8_t*)_time_text[_ps-1]);  //Show first edit position in menu bar

	/* Change to local time if currently UTC */
	if(m.UTC==1){
		m.UTC=0;utc_flag=TRUE;
		Show_time(9,0);  //Refresh display
	}

	/* Show F2 function in sub-menu based on menu mode */
	if(_mode==2 || _mode==3){
		sprintf((char*)ShowBuff,"IM=%d",m.IM);
		Gui_DrawFont_GBK16(48,112,C_WHITE,C_RED,ShowBuff);
		Clear_middle_display(0);
		Gui_DrawFont_GBK8(x,y+64,*_I_color,C_BLACK,(uint8_t*)"<-> Change   <|> Confirm");
		Gui_DrawFont_GBK8(x,y+74,*_I_color,C_BLACK,(uint8_t*)"Toggle IM for alarm INT mode");
	}
	if(_mode==4){
		Gui_DrawFont_GBK16(40,112,C_WHITE,C_BLUE,(uint8_t*)"RecRS");
		Clear_middle_display(0);
		Gui_DrawFont_GBK8(x,y+64,*_I_color,C_BLACK,(uint8_t*)"<-> Change   <|> Confirm");
		Gui_DrawFont_GBK8(x,y+74,*_I_color,C_BLACK,(uint8_t*)"RecRS > Reset TH/TL record");
	}

	/* Recall the stored WA_time for weekday alarm before enter the loop*/
	if(_mode==3){
		ReadByte(WA_time,0x02,&Time.minute);
		if(Time.minute>59 || Time.hour>23){  //If data out of range, make them zero at first use.
			Time.minute=0;Time.hour=0;
		}
	}

	HAL_Delay(500);  //Delay to avoid go into edit immediately

	do  //Loop for editing alarm or current date&time
	{
		if(Read_F1()){
			HAL_Delay(20);  //Avoid fake key press
			if(Read_F1()==TRUE){  //Abort change
				Draw_menu(m.MODE);  //Restore to caller sub-menu
				m.STATUS=0;  //Restore default
				save_exit=0;  //Set 0 to exit loop
			}
		}

		if(Read_F2() || Read_F3() || Read_PwrKey()){
			HAL_Delay(20);  //Avoid fake key press
			if(Read_F3()==TRUE){  //Caller press long enough to enter editing directly
				Blink_LED2();
				_ps++;  //Set pointer position
				if(_ps==_pe){
					_ps=Init_ps;  //Return to pointer start
				}
				Gui_DrawFont_GBK16(96,112,C_WHITE,C_RED,(uint8_t*)_time_text[_ps-1]);  //Show content in menu bar
			}
			if(Read_F2()==TRUE && (_mode==2 || _mode==3)){
				m.IM^=0x01;  //Toggle IM value
				sprintf((char*)ShowBuff,"IM=%d",m.IM);
				Gui_DrawFont_GBK16(48,112,C_WHITE,C_RED,ShowBuff);
				WriteByte(IM_flag,0x01,&m.IM);  //Save IM flag to RTC user memory 31H
			}
			if(Read_F2()==TRUE && _mode==4){
				Reset_THL_Record();Confirm_highlight();  //Reset temperature record
				Gui_DrawFont_GBK16(96,112,C_WHITE,C_BLUE,(uint8_t*)"   ");  //Clear text of "Set"
				Gui_DrawFont_GBK16(96,112,C_WHITE,C_RED,(uint8_t*)_time_text[_ps-1]);  //Redraw content
			}
			if(Read_PwrKey()==TRUE){  //Execute according to menu mode and return to upper menu
				if(_mode==2){
					SetAlarm(min_ALM|hor_ALM|day_ALM|mon_ALM,m.wa,m.IM,Time);  //Set alarm time and relevant flag
					ReadByte(EA,0x01,&m.ea); //Copy 0EH data to EA_flag
					Draw_menu(_mode);  //Restore to caller sub-menu
				}
				if(_mode==3){
					SetAlarm(min_ALM|hor_ALM|wek_ALM,m.wa,m.IM,Time);  //Set alarm time and relevant flag
					ReadByte(EA,0x01,&m.ea);  //Copy 0EH data to EA_flag
					Draw_menu(_mode);  //Restore to caller sub-menu
				}
				if(_mode==4){
					Time.second=0x00;  //Set second to zero
					WriteTime(Time);  //Write current date&time
					Draw_menu(_mode);  //Restore to caller sub-menu
				}
				m.STATUS=0;  //Restore default
				Confirm_highlight();
				save_exit=0;  //Set zero to exit loop
			}
		}
		if(Read_R() || Read_L()){  //Adjust the time value
			HAL_Delay(20);  //Avoid fake key press
			Blink_LED2();
			if(Read_R()==TRUE){  //Go up while key right
				  switch (_ps)
				  {
				  case 1:
					  Time.year++;if(Time.year == 100) Time.year=0;break;
				  case 2:
					  Time.month++;if(Time.month == 13) Time.month=1;break;
				  case 3:
				 	  Time.day++;if(Time.day == 32) Time.day=1;break;
				  case 4:
					  Time.hour++;if(Time.hour == 24) Time.hour=0;break;
				  case 5:
					  Time.minute++;if(Time.minute == 60) Time.minute=0;break;
				  case 6:
				   	  Time.week++;if(Time.week == 7) Time.week=0;break;
				  default:
					  return;
				  }
			  }
			  if(Read_L()==TRUE){  //Go down when key left
				  switch (_ps)
				  {
				  case 1:
					  Time.year--;if(Time.year == 255) Time.year=99;break;
				  case 2:
					  Time.month--;if(Time.month == 0) Time.month=12;break;
				  case 3:
				 	  Time.day--;if(Time.day == 0) Time.day=31;break;
				  case 4:
					  Time.hour--;if(Time.hour == 255) Time.hour=23;break;
				  case 5:
				   	  Time.minute--;if(Time.minute == 255) Time.minute=59;break;
				  case 6:
				   	  Time.week--;if(Time.week == 255) Time.week=6;break;
				  default:
					  return;
				  }
			  }
		}
		Show_time(1,_ps);  //Refresh the change of the selection
	}while(save_exit!=0);  //End while loop and return to caller
	if(utc_flag==TRUE){
		m.UTC=1;utc_flag=FALSE;  //If originally UTC, restore it
	}
	Show_time(9,0);  //Refresh display
	HAL_Delay(500);  //Delay to avoid further key press trigger ESC to root menu
}

void Set_repeat_alarm(void)
{
	Time_Def _Time;ReadTime(&_Time);
	_Time.minute=0x00;  //Alarm at 00 minute
	SetAlarm(min_ALM,m.wa,m.IM,_Time);  //Enable alarm for minute only
	ReadByte(EA,0x01,&m.ea);  //Read enable alarm data at 0EH to runtime variable ea
	Confirm_highlight();
}

void Weekday_setting(void)
{
	/* initialization */
	uint8_t save_return=0;  //Press F3 to save and return to previous sub-menu
	uint8_t k,i,_ps,_pe;
	_ps=0;_pe=6;
	ReadByte(WA,0x01,&m.wa); //Copy 0AH weekday alarm data
	Draw_title(disable);  //Clear title before loop
	Clear_middle_display(0);
	/* Show instruction */
	Gui_DrawFont_GBK8(60,0,C_CYAN,C_BLACK,(uint8_t*)"<-> Select weekday");
	Gui_DrawFont_GBK8(60,8,C_CYAN,C_BLACK,(uint8_t*)"<|> Toggle on/off");
	Gui_DrawFont_GBK8(60,16,C_CYAN,C_BLACK,(uint8_t*)"Press Set to save");

	do
	{
		Gui_DrawFont_GBK8(x,y+64,C_WHITE,C_BLACK,(uint8_t*)"Sun Mon Tue Wed Thr Fri Sat WA");
		for(i=0;i<=6;i++){
			k = m.wa>>i;  //Get the status of weekday bit setting
			if((k &= 0x01)==1) {Gui_DrawFont_GBK8(x+i*20,y+74,C_RED,C_BLACK,(uint8_t*)"-*-");}
			else {Gui_DrawFont_GBK8(x+i*20,y+74,C_RED,C_BLACK,(uint8_t*)"   ");}
		}
		sprintf((char*)ShowBuff,"%02X",m.wa);  //Show value to be stored in 0AH
	  	Gui_DrawFont_GBK8(x+140,y+74,C_BLUE,C_BLACK,ShowBuff);
	  	Gui_DrawFont_GBK24(8,0,C_RED,C_BLACK,(uint8_t*)_week_text[_ps]);  //Show selected weekday at title

		if(Read_R() || Read_L()){  //Selection of weekday
			HAL_Delay(100);  //Avoid go too fast
			if(Read_R()==TRUE) {_ps++;if(_ps>_pe) _ps=0;}  //Go upward
			if(Read_L()==TRUE) {_ps--;if(_ps==255) _ps=_pe;}  //Go downward
		}

		if(Read_F3() || Read_PwrKey()){  //Manipulate the change
			if(Read_PwrKey()==TRUE){  //Toggle enable/disable for the selected weekday
				  switch (_ps+1)
				  {
				  case 1:
					  m.wa ^= 0x01;break;
				  case 2:
					  m.wa ^= 0x02;break;
				  case 3:
					  m.wa ^= 0x04;break;
				  case 4:
					  m.wa ^= 0x08;break;
				  case 5:
					  m.wa ^= 0x10;break;
				  case 6:
					  m.wa ^= 0x20;break;
				  case 7:
					  m.wa ^= 0x40;break;
				  default:
					  return;
				  }
				  HAL_Delay(300);  //Delay to avoid further key press trigger again
			}
			if(Read_F3()==TRUE){  //Save change
				WriteByte(WA,0x01,&m.wa);  //Update setting to 0AH
				Confirm_highlight();Draw_title(enable);
				save_return=1;  //Exit loop
			}
		}
	}while(Read_F1()==FALSE && save_return==0);
	Draw_title(enable);  //Reset title after loop
	Draw_menu(3);  //Redraw sub-menu
}

void Count_down(void)
{
	  /* initialization */
	  uint8_t _ps,_pe;
	  _ps=1;_pe=3;
	  uint8_t CD_ctrl=0;  // control the count down display
	  old_second = T_1000ms;
	  Clear_middle_display(1);Draw_title(disable);
	  Gui_DrawFont_GBK16(0,112,C_LIME,C_BLUE,(uint8_t*)"ESC  Start Reset Set");  // show key function
	  Gui_DrawFont_GBK24(12,0,C_LIME,C_BLUE,(uint8_t*)"Count Down");
	  Gui_DrawFont_GBK8(x,y+54,C_CYAN,C_BLACK,(uint8_t*)"Press Set to change time.");
	  Gui_DrawFont_GBK8(x,y+64,C_CYAN,C_BLACK,(uint8_t*)"<-> Adjust time   <|> Save");
	  Gui_DrawFont_GBK8(x,y+74,C_CYAN,C_BLACK,(uint8_t*)"Press Finsh / Reset to restart.");
	  ReadByte(CD_value,0x03,&cd.ss);  // copy cd value at 32H to variable hh:mm:ss
	  cd.ref_val = T_1000ms;
	  cd.init_val = cd.hh*3600 + cd.mm*60 + cd.ss;
	  sprintf((char*)ShowBuff,"%02d:%02d:%02d",cd.hh,cd.mm,cd.ss);  //Show the previous CD value
	  Gui_DrawFont_GBK24(x+8,y,C_GRAY,C_BLACK,ShowBuff);

	  do
	  {
		  /* Show CTR2 status, 0x74 = enable, 0x70 = disable */
		  uint8_t buf=0;
		  ReadByte(CTR2,0x01,&buf);
		  sprintf((char*)ShowBuff,"%02X",buf);
		  Gui_DrawFont_GBK8(x+136,y+54,C_SNOW,C_RED,ShowBuff);
		  
		  /* Show the count down process only ref_val not smaller than system Second value */
		  if(cd.ref_val >= T_1000ms && CD_ctrl==1 && T_1000ms > old_second){
			  old_second = T_1000ms;
			  cd.dis_val = cd.ref_val - T_1000ms;
			  cd.ss = cd.dis_val%60;
			  cd.mm = cd.dis_val/60%60;
			  cd.hh = cd.dis_val/60/60;
			  sprintf((char*)ShowBuff,"%02d:%02d:%02d",cd.hh,cd.mm,cd.ss);
			  Gui_DrawFont_GBK24(x+8,y,C_GREEN,C_BLACK,ShowBuff);
		  }

		  /* Beep and reset when count down finished */
		  if(Read_RTC_int()==FALSE){
			  SetCountDown(disable, &cd);  //Stop count down
			  CD_ctrl=0;  //Reset control of CD show
			  Gui_DrawFont_GBK16(40,112,C_SNOW,C_RED,(uint8_t*)"Finsh");
			  Beep(8);
			  do{
				  HAL_Delay(50);  //Kill time
			  }while(Read_F2()==0);  //Hold till key press to retrieve count down value
			  ReadByte(CD_value,0x03,&cd.ss);
			  sprintf((char*)ShowBuff,"%02d:%02d:%02d",cd.hh,cd.mm,cd.ss);
			  Gui_DrawFont_GBK24(x+8,y,C_GRAY,C_BLACK,ShowBuff);  //Restore display to stored value
			  Gui_DrawFont_GBK16(40,112,C_LIME,C_BLUE,(uint8_t*)"Start");  //Redraw menu
			  HAL_Delay(300);  //Avoid too long key press trigger again
		  }

		  /* Toggle count down process */
		  if(Read_F2()){
			  Blink_LED2();
			  HAL_Delay(50);  //Avoid fake key press
			  if(Read_F2()==TRUE){
	  			  CD_ctrl^=0x01;
	  			  if(CD_ctrl==1){
	  				  Gui_DrawFont_GBK16(40,112,C_SNOW,C_RED,(uint8_t*)"Pause");
	  			  	  cd.IM = 1;  // set periodic INT
	  			  	  cd.d_clk = S_1s;  // count down clock frequency
	  			  	  cd.init_val = cd.hh*3600 + cd.mm*60 + cd.ss;  //Count down value based on display value
	  			  	  /* Make ref_val by adding cd.init_val with system Second value*/
	  			  	  cd.ref_val = cd.init_val + T_1000ms;
	  			  	  SetCountDown(enable, &cd);  //Start count down
	  			  }
	  			  else{
	  				  Gui_DrawFont_GBK16(40,112,C_LIME,C_BLUE,(uint8_t*)"Start");
	  				  SetCountDown(disable, &cd);  //Stop count down
	  			  }
			  }
			  HAL_Delay(300);  //Avoid too long key press trigger again
		  }

		  /* Restore count down value */
		  if(Read_F3()){
			  Blink_LED2();
			  HAL_Delay(50);  //Avoid fake key press
			  if(Read_F3()==TRUE && CD_ctrl==1){
				  CD_ctrl=0;  //Set no update
				  SetCountDown(disable, &cd);
				  ReadByte(CD_value,0x03,&cd.ss);
				  sprintf((char*)ShowBuff,"%02d:%02d:%02d",cd.hh,cd.mm,cd.ss);
				  Gui_DrawFont_GBK24(x+8,y,C_GRAY,C_BLACK,ShowBuff);  //Restore and show initial value
				  Gui_DrawFont_GBK16(40,112,C_LIME,C_BLUE,(uint8_t*)"Start");  //Redraw menu
			  }
			  HAL_Delay(300);  //Avoid too long key press trigger again
		  }

		  /* Adjust hh:mm:ss value and save a copy to 32H for next call */
		  if(Read_F4()==1 && CD_ctrl!=1){
			  Blink_LED2();
			  ReadByte(CD_value,0x03,&cd.ss);  //Get stored value
			  HAL_Delay(300);  //Avoid too long key press trigger again

			  do
			  {
				  /* display the set value and pointer */
				  if(_ps==1) Gui_DrawFont_GBK24(x+8,y+24,C_RED,C_BLACK,(uint8_t*)"      ss");
				  if(_ps==2) Gui_DrawFont_GBK24(x+8,y+24,C_RED,C_BLACK,(uint8_t*)"   mm   ");
				  if(_ps==3) Gui_DrawFont_GBK24(x+8,y+24,C_RED,C_BLACK,(uint8_t*)"hh      ");
				  sprintf((char*)ShowBuff,"%02d:%02d:%02d",cd.hh,cd.mm,cd.ss);
				  Gui_DrawFont_GBK24(x+8,y,C_YELLOW,C_BLACK,ShowBuff);

				  if(Read_F4()){
					  Blink_LED2();
					  HAL_Delay(50);  //Avoid fake key press
					  if(Read_F4()==TRUE) {_ps++;if(_ps>_pe) _ps=1;}
					  HAL_Delay(300);  //Avoid too long key press trigger again
				  }

				  if(Read_R() || Read_L()){  // change time value
					  Blink_LED2();
					  HAL_Delay(50);  //Avoid go too fast
					  if(Read_R()==TRUE){  // go up while key right
						  switch (_ps)
						  {
						  case 1:
							  cd.ss++;if(cd.ss == 60) cd.ss=0;break;
						  case 2:
							  cd.mm++;if(cd.mm == 60) cd.mm=0;break;
						  case 3:
							  cd.hh++;if(cd.hh == 24) cd.hh=0;break;
						  default:
							  return;
						  }
					  }
					  if(Read_L()==TRUE){  // go down when key left
						  switch (_ps)
						  {
						  case 1:
							  cd.ss--;if(cd.ss == 255) cd.ss=59;break;
						  case 2:
							  cd.mm--;if(cd.mm == 255) cd.mm=59;break;
						  case 3:
							  cd.hh--;if(cd.hh == 255) cd.hh=23;break;
						  default:
							  return;
						  }
					  }
				  }
			  }while(Read_PwrKey()==0);  //loop end when press key
			  Blink_LED2();
			  WriteByte(CD_value,0x03,&cd.ss);  //Save count down value to 32H for next call
			  Gui_DrawFont_GBK24(x+8,y+24,C_RED,C_BLACK,(uint8_t*)"        ");  //Clear the pointer display
			  Gui_DrawFont_GBK24(x+8,y,C_GRAY,C_BLACK,ShowBuff);  //Redraw the color of display value
		  }
	  }while(Read_F1()==FALSE);  //Main loop end and return to caller
	  Blink_LED2();
	  Clear_middle_display(1);Draw_title(enable);ClearAlarm(0,m.IM);  //Reset
	  Draw_menu(10);  //Show key function
	  Show_time(9,0);  //Redraw time digit and temperature
}

void Stop_watch(void)
{
	  /* initialization */
	  Clear_middle_display(1);Draw_title(disable);
	  Gui_DrawFont_GBK16(0,112,C_SNOW,C_BLUE,(uint8_t*)"ESC  Start Reset SW ");  // show key function
	  Gui_DrawFont_GBK24(12,0,C_SNOW,C_BLUE,(uint8_t*)"STOP WATCH");
	  uint8_t SW_ctrl=0;  //Control the stop watch display, 1=run
	  cd.hh=0x00,cd.mm=0x00;cd.ss=0x00;cd.ms=0x00;cd.dis_val=0x00;  //Reset all to zero
#if SW_Method==1
			  sprintf((char*)ShowBuff,"%02d:%02d:%02d",cd.hh,cd.mm,cd.ss);
			  Gui_DrawFont_GBK8(120,y+8,C_CYAN,C_BLACK,(uint8_t*)"1s");
#else
			  sprintf((char*)ShowBuff,"%02d:%02d:%02d",cd.mm,cd.ss,cd.ms);
			  Gui_DrawFont_GBK8(120,y+8,C_CYAN,C_BLACK,(uint8_t*)"1/100s");
#endif
			  Gui_DrawFont_GBK24(x+8,y,C_SILVER,C_BLACK,ShowBuff);  //Show initial counter
	  uint8_t _count=0;  //Stop watch count
	  uint16_t _SW_color[3];  //Stop watch color
	  _SW_color[0]=C_DARK_CYAN;
	  _SW_color[1]=C_CYAN;
	  _SW_color[2]=C_LIGHT_CYAN;

	  do
	  {
#if SW_Method==1
		  if(T_1000ms > cd.ref_val && SW_ctrl==1)
#else
		  if(T_100ms > cd.ref_val && SW_ctrl==1)
#endif
		  {
#if SW_Method==1
			  cd.dis_val = T_1000ms - cd.ref_val;
			  cd.ss = cd.dis_val%60;
			  cd.mm = cd.dis_val/60%60;
			  cd.hh = cd.dis_val/60/60;
			  sprintf((char*)ShowBuff,"%02d:%02d:%02d",cd.hh,cd.mm,cd.ss);
			  Gui_DrawFont_GBK24(x+8,y,C_SILVER,C_BLACK,ShowBuff);  //Update the counter display
#else
			  cd.dis_val = T_100ms - cd.ref_val;
			  cd.ms = (cd.dis_val%10)*10;
			  cd.ss = cd.dis_val/10%60;
			  cd.mm = cd.dis_val/10/60;
			  sprintf((char*)ShowBuff,"%02d:%02d:%02d",cd.mm,cd.ss,cd.ms);
			  Gui_DrawFont_GBK24(x+8,y,C_SILVER,C_BLACK,ShowBuff);  //Update the counter display
#endif
		  }

		  if(Read_F3()){  // reset to zero
			  Blink_LED2()  // blink LED2 once while key press
			  HAL_Delay(50);  //Avoid fake key press
			  if(Read_F3()==TRUE){
				  Clear_middle_display(1);
				  SW_ctrl=0;cd.hh=0x00,cd.mm=0x00;cd.ss=0x00;cd.ms=0x00;cd.dis_val=0x00;_count=0;
#if SW_Method==1
				  sprintf((char*)ShowBuff,"%02d:%02d:%02d",cd.hh,cd.mm,cd.ss);
				  Gui_DrawFont_GBK8(120,y+8,C_CYAN,C_BLACK,(uint8_t*)"1s");
#else
				  sprintf((char*)ShowBuff,"%02d:%02d:%02d",cd.mm,cd.ss,cd.ms);
				  Gui_DrawFont_GBK8(120,y+8,C_CYAN,C_BLACK,(uint8_t*)"1/100s");
#endif
				  Gui_DrawFont_GBK24(x+8,y,C_SILVER,C_BLACK,ShowBuff);  //Show initial counter
				  Gui_DrawFont_GBK16(40,112,C_SNOW,C_BLUE,(uint8_t*)"Start");  //Redraw menu
			  }
		  }

		  if(Read_F2()){  // start or pause count down process
			  Blink_LED2()  // blink LED2 once while key press
			  if(Read_F2()==TRUE){
	  			  SW_ctrl^=0x01;
	  			  if(SW_ctrl==1){
	  				  Gui_DrawFont_GBK16(40,112,C_SNOW,C_RED,(uint8_t*)"Pause");
#if SW_Method==1
	  				  cd.ref_val = T_1000ms - cd.dis_val;
#else
	  				  cd.ref_val = T_100ms - cd.dis_val;
#endif
	  			  }
	  			  else{
	  				  Gui_DrawFont_GBK16(40,112,C_SNOW,C_BLUE,(uint8_t*)"Start");
	  			  }
#if SW_Method==1
			  sprintf((char*)ShowBuff,"%02d:%02d:%02d",cd.hh,cd.mm,cd.ss);
#else
			  sprintf((char*)ShowBuff,"%02d:%02d:%02d",cd.mm,cd.ss,cd.ms);
#endif
			  Gui_DrawFont_GBK24(x+8,y,C_SILVER,C_BLACK,ShowBuff);  //Show the last counter value
			  }
			  HAL_Delay(300);  //Avoid too long key press trigger again
		  }

		  if(Read_PwrKey() || Read_F4()){
			  if(Read_PwrKey()==TRUE || Read_F4()==TRUE){
				  if(_count>2){_count=0;}  //Show 3 stop watch
#if SW_Method==1
				  sprintf((char*)ShowBuff,"Count %1d = %02d:%02d:%02d",_count+1,cd.hh,cd.mm,cd.ss);
#else
				  sprintf((char*)ShowBuff,"Count %1d = %02d:%02d:%02d",_count+1,cd.mm,cd.ss,cd.ms);
#endif
				  Gui_DrawFont_GBK16(x+8,y+30+_count*16,_SW_color[_count],C_BLACK,ShowBuff);
				  _count++;
			  }
			  HAL_Delay(300);  //Avoid too long key press trigger again
		  }
	  }while(Read_F1()==0);
	  Clear_middle_display(1);Draw_title(enable);ClearAlarm(0,m.IM);
	  Draw_menu(10);  //Show key function
	  Show_time(9,0);  //Redraw time digit and temperature
}

void Freq_out(void)
{
	  /* initialization */
	  uint8_t f_out,_ps,_pe,toggle;
	  f_out=0;
	  ReadByte(Freq_value, 0x01, &f_out);
	  _ps=f_out+1;_pe=15;toggle=1;  //Load the stored freq_out value in 35H and set it start position
	  Draw_title(disable);  //Clear title
	  Clear_middle_display(1);  //Clear time display area
	  Gui_DrawFont_GBK24(0,0,C_YELLOW,C_DARK_CYAN,(uint8_t*)"Freq:        ");
	  Gui_DrawFont_GBK24(70,0,C_YELLOW,C_DARK_CYAN,(uint8_t*)_freq_text[_ps-1]);  //Display stored freq_out value
	  Gui_DrawFont_GBK16(0,y+4,C_CYAN,C_BLACK, (uint8_t*)"RTC frequency output");
	  Gui_DrawFont_GBK16(0,y+20,C_CYAN,C_BLACK,(uint8_t*)"via INT out (pin 7) ");
	  Gui_DrawFont_GBK16(0,y+36,C_CYAN,C_BLACK,(uint8_t*)"==> EXT port pin 14 ");
	  Gui_DrawFont_GBK8(0,y+64,C_WHITE,C_BLACK,(uint8_t*)"<-> Select Freq   <|> Output");
	  Gui_DrawFont_GBK8(0,y+74,C_WHITE,C_BLACK,(uint8_t*)"Press ESC return to caller");
	  Gui_DrawFont_GBK16(0,112,C_YELLOW,C_DARK_CYAN, (uint8_t*)"Esc                 ");  //Show menu bar

	  do
	  {
		  if(Read_R() || Read_L()){  // change frequency value
			  Blink_LED2();
			  if(Read_R()==TRUE) {_ps++;if(_ps>_pe) _ps=1;}  //Go upward
			  if(Read_L()==TRUE) {_ps--;if(_ps<1) _ps=15;}  //Go downward
			  Gui_DrawFont_GBK24(70,0,C_YELLOW,C_DARK_CYAN,(uint8_t*)"      ");  //Clear text
			  Gui_DrawFont_GBK24(70,0,C_YELLOW,C_DARK_CYAN,(uint8_t*)_freq_text[_ps-1]);  //Show selected freq_out value
			  HAL_Delay(300);  //Delay to avoid further key press trigger again
		  }

		  if(Read_PwrKey()){  //Toggle frequency out
			  if(Read_PwrKey()==TRUE){
				  toggle^=0x01;
				  if(toggle!=0){
					  SetFreq(enable, F_0Hz);  //Set value of F_0Hz so that no output
					  Gui_DrawFont_GBK16(128,112,C_YELLOW,C_DARK_CYAN,(uint8_t*)"    ");
				  }
				  else{
					  SetFreq(enable, F_0Hz+_ps);  //Set value as display
					  Gui_DrawFont_GBK16(128,112,C_WHITE,C_RED,(uint8_t*)"*OUT");
					  f_out=_ps-1;
					  WriteByte(Freq_value, 0x01, &f_out);  //Saved the current freq_out value to 35H for next call
				  }
				  HAL_Delay(300);  //Delay to avoid further key press trigger the toggle action
			  }
		  }
	  }while(Read_F1()==FALSE); //Return to previous sub-menu
	  f_out=_ps-1;
	  WriteByte(Freq_value, 0x01, &f_out);  //Saved the current freq_out value to 35H for next call
	  SetFreq(disable, F_0Hz);  //Shut off frequency out before return to caller
	  Draw_title(enable);
	  Clear_middle_display(1);
	  Draw_menu(5);Show_time(9,0);  //Redraw the time display and sub-menu
	  HAL_Delay(500);  //Delay to avoid further key press trigger ESC to root menu
}

void Display_rtc_conf(uint8_t *_ps, Temper_Bat_Def* _TnB)
{
	char PS;  //Display '*' as indicator
	char* BATTERY;
	char* ALARM;  //String for resolution text

  	switch (_TnB->charge)
  	{
  	case DIS_charge:
  		BATTERY = "Disable   ";
  		break;
  	case EN_charge_10k:
  		BATTERY = "Charge 10k";
  		break;
  	case EN_charge_5k:
  		BATTERY = "Charge 5k ";
  		break;
  	case En_charge_2k:
  		BATTERY = "Charge 2k ";
  		break;
  	default:
  		BATTERY = "Invalid   ";
  		break;
  	}

  	switch (_TnB->ctr4)
  	{
  	case ALARM_DISABLE:
  		ALARM = "Disable  ";
  		break;
  	case TL_EN:
  		ALARM = "Temp low ";
  		break;
  	case TH_EN:
  		ALARM = "Temp high";
  		break;
  	case BL_EN:
  		ALARM = "Bat low  ";
  		break;
  	case BH_EN:
  		ALARM = "Bat high ";
  		break;
  	default:
  		ALARM = "Invalid   ";
  		break;
  	}

  	/* Show battery register information */
	sprintf((char*)ShowBuff,"Bat_I2C: %02Xh   0=DIS 1=EN",TnB.batiic);
	Gui_DrawFont_GBK8(8, 0, C_ORANGE, C_BLACK, ShowBuff);

  	/* Show which parameter is selected and its value */
	if(*_ps==1){PS='*';}else{PS=' ';}
	sprintf((char*)ShowBuff,"%cBat: %s",PS,BATTERY);
	Gui_DrawFont_GBK16(0, 10, C_ORANGE, C_BLACK, ShowBuff);
	if(*_ps==2){PS='*';}else{PS=' ';}
	sprintf((char*)ShowBuff,"%cALm Type: %s",PS,ALARM);
	Gui_DrawFont_GBK16(0, 30, C_CYAN, C_BLACK, ShowBuff);
	if(*_ps==3){PS='*';}else{PS=' ';}
	sprintf((char*)ShowBuff,"%cAlarm TH: %3d'C",PS,_TnB->high);
	Gui_DrawFont_GBK16(0, 50, C_CYAN, C_BLACK, ShowBuff);
	if(*_ps==4){PS='*';}else{PS=' ';}
	sprintf((char*)ShowBuff,"%cAlarm TL: %3d'C",PS,_TnB->low);
	Gui_DrawFont_GBK16(0, 70, C_CYAN, C_BLACK, ShowBuff);

	/* Indicate if RTC battery charge mode is set */
	uint8_t data;
	ReadByte(Chg_MG,0x01,&data);  //Check register of battery charge status
	if(data==0x03){
		Gui_DrawFont_GBK8(x+70,y+74,C_GREEN,C_BLACK,(uint8_t*)"BatChg= disable");
	}
	else{
		Gui_DrawFont_GBK8(x+70,y+74,C_RED,C_BLACK,(uint8_t*)"BatChg= enable ");
	}
}

void TnB_adjust_conf(void)
{
	Lcd_Clear(0, 0, 159, 127, C_BLACK);  //Clear whole display
	uint8_t ps=1,pe=4;
	int8_t bat_charge_set,alarm_set;
	Temper_Bat_Def Conf;

	/* Display menu */
	Gui_DrawFont_GBK16(0, 112, C_YELLOW, C_BLACK, (uint8_t*)" ESC      Select Set");

	Show_reg_info();  //Display register data above menu bar
	TnB_GetValue(&Conf);  //Get configuration

	do
	{
		Display_rtc_conf(&ps,&Conf);  //Update the change

		switch(Conf.charge)  //Convert actual value to 0~3 for adjusting
	    {
        case DIS_charge:
        	bat_charge_set = 0;
        	break;
    	case EN_charge_10k:
    		bat_charge_set = 1;
            break;
        case EN_charge_5k:
        	bat_charge_set = 2;
            break;
        case En_charge_2k:
        	bat_charge_set = 3;
            break;
        default:
        	bat_charge_set = 0;  //Avoid data error
            break;
	    }

		switch(Conf.ctr4)  //Convert actual value to 0~3 for adjusting
	    {
        case ALARM_DISABLE:
        	alarm_set = 0;
        	break;
    	case TL_EN:
    		alarm_set = 1;
            break;
        case TH_EN:
        	alarm_set = 2;
            break;
        case BL_EN:
        	alarm_set = 3;
            break;
        case BH_EN:
        	alarm_set = 4;
            break;
        default:
        	alarm_set = 0;  //Avoid data error
            break;
	    }

		if(Read_F3()){
			HAL_Delay(100);  //Avoid go too fast
			if(Read_F3()==1) {ps++;if(ps>pe) ps=1;}  //Selection pointer for below case switch
		}

		if(Read_R()){  //Go up while key right
			HAL_Delay(50);  //Avoid go too fast
			if(Read_R()==1){
				switch (ps)
				{
				case 1:
					bat_charge_set++;if(bat_charge_set > 3) bat_charge_set = 0;
					break;
				case 2:
					alarm_set++;if(alarm_set > 4) alarm_set = 0;
					break;
				case 3:
					Conf.high++;if(Conf.high > 85) Conf.high = 5;
					break;
				case 4:
					Conf.low++;if(Conf.low > 5) Conf.low = -40;
					break;
				default:
					return;
				}
			}
		}

		if(Read_L()){  //Go down when key left
			HAL_Delay(100);  //Avoid go too fast
			if(Read_L()==1){
				switch (ps)
				{
				case 1:
					bat_charge_set--;if(bat_charge_set < 0) bat_charge_set = 3;
					break;
				case 2:
					alarm_set--;if(alarm_set < 0) alarm_set = 4;
					break;
				case 3:
					Conf.high--;if(Conf.high < 5) Conf.high = 85;
					break;
				case 4:
					Conf.low--;if(Conf.low < -40) Conf.low = 5;
					break;
				default:
					return;
				}
			}
		}

	    switch(bat_charge_set)  //Set value of configuration after change
	    {
    	case 0:
    		Conf.charge = DIS_charge;
            break;
        case 1:
        	Conf.charge = EN_charge_10k;
            break;
        case 2:
        	Conf.charge = EN_charge_5k;
            break;
        case 3:
        	Conf.charge = En_charge_2k;
            break;
        default:
        	Conf.charge = DIS_charge;  //Avoid data error
            break;
	    }

	    switch(alarm_set)  //Set value of configuration after change
	    {
    	case 0:
    		Conf.ctr4 = ALARM_DISABLE;
            break;
        case 1:
        	Conf.ctr4 = TL_EN;
            break;
        case 2:
        	Conf.ctr4 = TH_EN;
            break;
        case 3:
        	Conf.ctr4 = BL_EN;
            break;
        case 4:
        	Conf.ctr4 = BH_EN;
            break;
        default:
        	Conf.ctr4 = ALARM_DISABLE;  //Avoid data error
            break;
	    }

		if(Read_F4()){  //Save configuration and return to caller
			HAL_Delay(500);  //Make sure no wrong press
			if(Read_F4()==1){
				TnB_SetValue(&Conf);
				Gui_DrawFont_GBK16(0, 112, C_YELLOW, C_RED, (uint8_t*)" Saving to register ");  //Show action done
				HAL_Delay(1000);  //Delay for visual effect
				Gui_DrawFont_GBK16(0, 112, C_YELLOW, C_BLACK, (uint8_t*)" ESC     Select  Set");  //Redraw menu
			}
		}
	}while(Read_F1()==FALSE);  //Exit to caller menu
	Lcd_Clear(0, 0, 159, 127, C_BLACK);  //Clear whole display
	Draw_title(enable);
	Show_time(9,0);  //Refresh display
	Draw_menu(1);  //Redraw the caller sub-menu
}
