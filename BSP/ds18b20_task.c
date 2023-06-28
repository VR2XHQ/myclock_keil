/*
 * ds18b20_task.c
 *
 *  Created on: Nov 3, 2021
 *      Author: Ben Yuen
 */

#include "ds18b20_task.h"

uint8_t	ShowBuff[48];  //Buff for content display

void Front_page(void)
{
	uint8_t y;
	y = 11;
	Lcd_Clear(0, 0, 159, 127, C_BLACK);
	Gui_DrawFont_GBK16(0, 0, C_CORN_FLOWER_BLUE, C_BLACK, (uint8_t*)"DS18B20 function set");
	Gui_DrawFont_GBK8(0, 20, C_CORN_FLOWER_BLUE, C_BLACK, (uint8_t*)"designed for HamCube by VR2XHQ");
	Gui_DrawFont_GBK8(0, y*3, C_GREEN_YELLOW, C_BLACK, (uint8_t*)"F1 = ESC");
	Gui_DrawFont_GBK8(0, y*4, C_GREEN_YELLOW, C_BLACK, (uint8_t*)"F2 = Search ROM");
	Gui_DrawFont_GBK8(0, y*5, C_GREEN_YELLOW, C_BLACK, (uint8_t*)"F3 = Search ALARM");
	Gui_DrawFont_GBK8(0, y*6, C_GREEN_YELLOW, C_BLACK, (uint8_t*)"F4 = Adjust configuration");
	Gui_DrawFont_GBK8(0, y*7, C_GREEN_YELLOW, C_BLACK, (uint8_t*)"<--  = Show temperature list");
	Gui_DrawFont_GBK8(0, y*8, C_GREEN_YELLOW, C_BLACK, (uint8_t*)"-->  = Thermometer & Curve");
	Gui_DrawFont_GBK8(0, y*9, C_GREEN_YELLOW, C_BLACK, (uint8_t*)"<|>  = Default ROM information");
	Gui_DrawFont_GBK16(16, 112, C_YELLOW, C_BLACK, (uint8_t*)"F1   F2   F3   F4");
}

void ShowTemper(uint8_t _romNum)  //Display temperature with decimal point of 4 digits and negative value
{
	int8_t data[5];
	if(Temper_Var<0){  //If negative value
		data[0]=Temper_Var/10000;
		data[1]=-Temper_Var%10000/1000;  // decimal point
		data[2]=-Temper_Var%1000/100;
		data[3]=-Temper_Var%100/10;
		data[4]=-Temper_Var%10;
	}
	else{
		data[0]=Temper_Var/10000;
		data[1]=Temper_Var%10000/1000;  // decimal point
		data[2]=Temper_Var%1000/100;
		data[3]=Temper_Var%100/10;
		data[4]=Temper_Var%10;
	}
	sprintf((char *)ShowBuff,"ROM %d  %d.%d%d%d%d'C",_romNum,data[0],data[1],data[2],data[3],data[4]);
	Gui_DrawFont_GBK16(10, 0+15*_romNum, C_LIGHT_BLUE, C_BLACK, ShowBuff);
	Gui_DrawFont_GBK8(10, 120, C_YELLOW, C_BLACK, (uint8_t*)"ESC     Temperature List");
}

void ds18b20_write_conf(DS18B20_RomCode_t *pRom, DS18B20_ConfigInfo_t *ConfigInfo)
{
	/* Show the RomCode of device to be wrote configuration */
	Gui_DrawFont_GBK16(0, 4, C_FOREST_GREEN, C_BLACK, (uint8_t*)"Configuration saved");
	sprintf((char *)ShowBuff,"Rom=%d PID=%02X CRC=%02X",0,pRom->familyCode,pRom->crcCode);
	Gui_DrawFont_GBK8(0, 80, C_CYAN, C_BLACK, ShowBuff);
	for(uint8_t i=6;0<i;i--){
		sprintf((char *)ShowBuff,"%02X",((uint8_t*)&pRom)[i]);
		Gui_DrawFont_GBK8(160-i*10, 80, C_CYAN, C_BLACK, ShowBuff);
	}

	DS18B20_SetConfig(pRom,ConfigInfo);
	/* Show configuration after set */
  	sprintf((char *)ShowBuff,"Alarm TL=%3d TH=%3d",ConfigInfo->alarmTL,ConfigInfo->alarmTH);
  	Gui_DrawFont_GBK16(0, 52, C_CRIMSON, C_BLACK, ShowBuff);
  	switch (ConfigInfo->resolution)
  	{
  	case resolution_9Bit:
  		sprintf((char *)ShowBuff,"%s",(uint8_t*)"Res= 9bits 0.5'C    ");
  		break;
  	case resolution_10Bit:
  		sprintf((char *)ShowBuff,"%s",(uint8_t*)"Res= 10bits 0.25'C  ");
  		break;
  	case resolution_11Bit:
  		sprintf((char *)ShowBuff,"%s",(uint8_t*)"Res= 11bits 0.125'C ");
  		break;
  	case resolution_12Bit:
  		sprintf((char *)ShowBuff,"%s",(uint8_t*)"Res= 12bits 0.0625'C");
  		break;
  	default:
  		break;
  	}
  	Gui_DrawFont_GBK16(0, 28, C_CRIMSON, C_BLACK, ShowBuff);
  	Gui_DrawFont_GBK16(8, 112, C_YELLOW, C_BLACK, (uint8_t*)"ESC");

  	do{
	}while(Read_F1()==0);  //Pause for action
}

void Display_adjust_conf(uint8_t *_ps, DS18B20_ConfigInfo_t* _Info_set)
{
	char PS;  //Display '*' as indicator
	char* RES;  //String for resolution text
	RES = "9bits ";  //Set value to avoid compile warning;

  	switch (_Info_set->resolution)
  	{
  	case resolution_9Bit:
  		RES = "9bits ";
  		break;
  	case resolution_10Bit:
  		RES = "10bits";
  		break;
  	case resolution_11Bit:
  		RES = "11bits";
  		break;
  	case resolution_12Bit:
  		RES = "12bits";
  		break;
  	default:
  		break;
  	}

  	/* Show which parameter is selected and its value */
	if(*_ps==1){PS='*';}else{PS=' ';}
	sprintf((char*)ShowBuff,"%c Resolution: %s",PS,RES);
	Gui_DrawFont_GBK16(0, 28, C_CYAN, C_BLACK, ShowBuff);
	if(*_ps==2){PS='*';}else{PS=' ';}
	sprintf((char*)ShowBuff,"%c Alarm TH: %3d'C",PS,_Info_set->alarmTH);
	Gui_DrawFont_GBK16(0, 52, C_CYAN, C_BLACK, ShowBuff);
	if(*_ps==3){PS='*';}else{PS=' ';}
	sprintf((char*)ShowBuff,"%c Alarm TL: %3d'C",PS,_Info_set->alarmTL);
	Gui_DrawFont_GBK16(0, 76, C_CYAN, C_BLACK, ShowBuff);
}

void ds18b20_adjust_conf(void)
{
	Lcd_Clear(0, 0, 159, 127, C_BLACK);
	uint8_t ps=1,pe=3,loop=1;
	int8_t res_set=0;
	DS18B20_ConfigInfo_t Info_set;

	uint8_t count = 1;  //Set 1 to search for single DS18B20 attached and its code is stored into RomID[0]
	int8_t status = DS18B20_SearchRom(RomID,&count);
	if(status!=0){  //If no device attached or found, don't run the loop
		loop = 0;
		Gui_DrawFont_GBK24(24, 40, C_RED, C_BLACK, (uint8_t*)"NO DEVICE");
		Gui_DrawFont_GBK24(24, 70, C_RED, C_BLACK, (uint8_t*)"=>=> EXIT");
		goto Exit;
	}
	/* Display menu */
	Gui_DrawFont_GBK16(0, 0, C_YELLOW, C_BLACK, (uint8_t*)"Adjust Configuration");
	Gui_DrawFont_GBK16(8, 112, C_YELLOW, C_BLACK, (uint8_t*)"ESC  F2  Select Set");
	Gui_DrawFont_GBK8(16, 100, C_SILVER, C_BLACK, (uint8_t*)"F2 > Check program timming");

	DS18B20_GetConfig(&RomID[0], &Info_set);
	while(loop==1)  //Loop for adjustment
	{
		Display_adjust_conf(&ps,&Info_set);  //Update the change

		switch(Info_set.resolution)  //Convert actual value to 0~3 for adjusting
	    {
    	case resolution_9Bit:
    		res_set = 0;
            break;
        case resolution_10Bit:
        	res_set = 1;
            break;
        case resolution_11Bit:
        	res_set = 2;
            break;
        case resolution_12Bit:
        	res_set = 3;
            break;
        default:
            break;
	    }

		if(Read_F2()){
			HAL_Delay(100);
			if(Read_F2()==1){
				DS18B20_check_us_timming(1);
				Gui_DrawFont_GBK16(48, 112, C_YELLOW, C_RED, (uint8_t*)"us");
				HAL_Delay(500);  //Avoid trigger key again
				do{
					HAL_Delay(50);  //Kill time
				}while(Read_F2()==0);
				DS18B20_check_us_timming(2);
				Gui_DrawFont_GBK16(48, 112, C_YELLOW, C_RED, (uint8_t*)"ms");
				HAL_Delay(500);  //Avoid trigger key again
				do{
					HAL_Delay(50);  //Kill time
				}while(Read_F2()==0);
				Gui_DrawFont_GBK16(8, 112, C_YELLOW, C_BLACK, (uint8_t*)"ESC  F2  Select Set");
			}
		}

		if(Read_F3()){
			HAL_Delay(100);
			if(Read_F3()==1) {ps++;if(ps>pe) ps=1;}  //Selection pointer for below case switch
		}

		if(Read_R()){  // go up while key right
			HAL_Delay(100);
			if(Read_R()==1){
				switch (ps)
				{
				case 1:
					res_set++;if(res_set > 3) res_set = 0;
					break;
				case 2:
					Info_set.alarmTH++;if(Info_set.alarmTH > 100) Info_set.alarmTH = 5;
					break;
				case 3:
					Info_set.alarmTL++;if(Info_set.alarmTL > 5) Info_set.alarmTL = -20;
					break;
				default:
					return;
				}
			}
		}

		if(Read_L()){  // go down when key left
			HAL_Delay(100);
			if(Read_L()==1){
				switch (ps)
				{
				case 1:
					res_set--;if(res_set < 0) res_set = 3;
					break;
				case 2:
					Info_set.alarmTH--;if(Info_set.alarmTH < 5) Info_set.alarmTH = 100;
					break;
				case 3:
					Info_set.alarmTL--;if(Info_set.alarmTL < -20) Info_set.alarmTL = 5;
					break;
				default:
					return;
				}
			}
		}

	    switch(res_set)  //Set value of configuration after change
	    {
    	case 0:
    		Info_set.resolution = resolution_9Bit;
            break;
        case 1:
        	Info_set.resolution = resolution_10Bit;
            break;
        case 2:
        	Info_set.resolution = resolution_11Bit;
            break;
        case 3:
        	Info_set.resolution = resolution_12Bit;
            break;
        default:
            break;
	    }

		if(Read_F4()){  // save configuration and return to caller
			HAL_Delay(200);
			if(Read_F4()==1){
				Lcd_Clear(0, 0, 159, 127, C_BLACK);
				ds18b20_write_conf(&RomID[0], &Info_set);
				loop=0;
			}
		}

		if(Read_F1()){  // do not save configuration and return to caller
			HAL_Delay(200);
			if(Read_F1()==1){
				loop=0;
			}
		}
	}

	Exit:
	HAL_Delay(1000);
	Lcd_Clear(0, 0, 159, 127, C_BLACK);
	Front_page();
}

void ds18b20_found_rom(uint8_t _count)
{
	Lcd_Clear(0, 0, 159, 127, C_BLACK);
	int8_t status = DS18B20_SearchRom(RomID,&_count);
	sprintf((char *)ShowBuff,"Found %d DS18B20     Press <|>",_count);
	Gui_DrawFont_GBK8(5, 120, C_CYAN, C_BLACK, ShowBuff);

	if(status == 0){
		for(uint8_t j=0;j<_count;j++){
			sprintf((char *)ShowBuff,"Rom[%d] ID=%02X CRC=%02X",j,RomID[j].familyCode,RomID[j].crcCode);
			Gui_DrawFont_GBK8(0, 0+j*12, C_ORANGE, C_BLACK, ShowBuff);
			for(uint8_t i=6;0<i;i--){
				sprintf((char *)ShowBuff,"%02X",((uint8_t*)&RomID[j])[i]);
				Gui_DrawFont_GBK8(160-i*10, 0+j*12, C_FOREST_GREEN, C_BLACK, ShowBuff);
			}
		}
	}
	else{
		Gui_DrawFont_GBK24(24, 40, C_RED, C_BLACK, (uint8_t*)"ERROR !!!");
		Gui_DrawFont_GBK24(24, 70, C_RED, C_BLACK, (uint8_t*)"==>> EXIT");
		goto Exit;
	}

	do{
	}while(Read_PwrKey()==0);  //Pause for action
	Lcd_Clear(0, 0, 159, 127, C_BLACK);

	DS18B20_ConvertTemperatureAll();
	HAL_Delay(500);
	for(uint8_t i=0;i<_count;i++){
#if method == 1
		DS18B20_ReadTemperature(&RomID[i], &temper_var);
		Temper_Var = (int32_t)(temper_var*10000);
#else
		DS18B20_ReadTemper(&RomID[i], &Temper_Var);
#endif
			ShowTemper(i);
	}

	do{
	}while(Read_F1()==0);  //Pause for action

	Exit:
	HAL_Delay(2000);
	Lcd_Clear(0, 0, 159, 127, C_BLACK);
	Front_page();
}

void ds18b20_found_alarm(uint8_t _count)
{
	Lcd_Clear(0, 0, 159, 127, C_BLACK);
	DS18B20_ConvertTemperatureAll();
	HAL_Delay(500);
	int8_t status = DS18B20_SearchAlarm(AlarmID,&_count);
	sprintf((char *)ShowBuff,"Found %d ALARM     Press <|>",_count);
	Gui_DrawFont_GBK8(5, 120, C_CYAN, C_BLACK, ShowBuff);

	if(status == 0){
		for(uint8_t j=0;j<_count;j++){
			sprintf((char *)ShowBuff,"Rom[%d] ID=%02X CRC=%02X",j,AlarmID[j].familyCode,AlarmID[j].crcCode);
			Gui_DrawFont_GBK8(0, 0+j*12, C_HOT_PINK, C_BLACK, ShowBuff);
			for(uint8_t i=6;0<i;i--){
				sprintf((char *)ShowBuff,"%02X",((uint8_t*)&AlarmID[j])[i]);
				Gui_DrawFont_GBK8(160-i*10, 0+j*12, C_FOREST_GREEN, C_BLACK, ShowBuff);
			}
		}
	}
	else{
		Gui_DrawFont_GBK24(24, 40, C_RED, C_BLACK, (uint8_t*)"NO ALARM");
		Gui_DrawFont_GBK24(24, 70, C_RED, C_BLACK, (uint8_t*)"==>> EXIT");
		goto Exit;
	}

	do{
	}while(Read_PwrKey()==0);  //Pause for action
	Lcd_Clear(0, 0, 159, 127, C_BLACK);

	DS18B20_ConvertTemperatureAll();
	HAL_Delay(500);
	for(uint8_t i=0;i<_count;i++){
#if method == 1
		DS18B20_ReadTemperature(&AlarmID[i], &temper_var);
		Temper_Var = (int32_t)(temper_var*10000);
#else
		DS18B20_ReadTemper(&AlarmID[i], &Temper_Var);
#endif
		ShowTemper(i);
	}

	do{
	}while(Read_F1()==0);  //Pause for action

	Exit:
	HAL_Delay(2000);
	Lcd_Clear(0, 0, 159, 127, C_BLACK);
	Front_page();
}

int8_t ds18b20_check(void)
{
	Lcd_Clear(0, 0, 159, 127, C_BLACK);
	Gui_DrawFont_GBK16(8, 112, C_YELLOW, C_BLACK, (uint8_t*)"ESC");

	if(DS18B20_if_parasite()==0){  //Check device power mode
		Gui_DrawFont_GBK8(50, 112, C_GREEN, C_BLACK, (uint8_t*)"Parasite power found");
	}
	else{
		Gui_DrawFont_GBK8(50, 112, C_RED, C_BLACK, (uint8_t*)"External power      ");
	}

	/* Check single ROM information which is stored into RomID[0] */
	RomID[0].romCode = 0;  //Reset content
	uint8_t count = 1;  //Set 1 to search for single DS18B20 attached and its code is stored into RomID[0]
	int8_t status = DS18B20_SearchRom(RomID,&count);
//	int8_t status = DS18B20_Read_romcode(&RomID[0]);  //Use this only when single DS18B20 attached
	sprintf((char *)ShowBuff,"Default ROM Info   Status= %d",status);
	Gui_DrawFont_GBK8(0, 4, C_SNOW, C_BLACK, ShowBuff);
	sprintf((char *)ShowBuff,"Rom[%d] ID=%02X CRC=%02X",0,RomID[0].familyCode,RomID[0].crcCode);
	Gui_DrawFont_GBK8(0, 20, C_CYAN, C_BLACK, ShowBuff);
	for(uint8_t i=6;0<i;i--){
		sprintf((char *)ShowBuff,"%02X",((uint8_t*)&RomID[0])[i]);
		Gui_DrawFont_GBK8(160-i*10, 20, C_CYAN, C_BLACK, ShowBuff);
	}

	switch(status)
	{
		case 0:  //Return 0, found ROM success
		{
	  	  	DS18B20_ConfigInfo_t Info_get = {0, 0, 0};  //Read data
	  	  	DS18B20_ConfigInfo_t Info_set = {0, 0, resolution_9Bit};  //Dummy data
	  	  	DS18B20_ConfigInfo_t Info_backup = {0, 0, 0};  //Backup data
	  	  	DS18B20_GetConfig(&RomID[0], &Info_get);  //Backup for restoration
	  	  	Info_backup = Info_get;

	  	  	DS18B20_SetConfig(&RomID[0], &Info_set);  //Write dummy data for verification
	  	  	DS18B20_GetConfig(&RomID[0], &Info_get);  //Read again for verification
	  	  	/* If not equal, probably can't set the configuration */
	  	  	if(Info_get.alarmTL != Info_set.alarmTL || Info_get.alarmTH != Info_set.alarmTH || Info_get.resolution != Info_set.resolution){
	  	  		Gui_DrawFont_GBK8(0, 36, C_RED, C_BLACK, (uint8_t*)"Device is not fully featured   ");
	  	  	}
	  	  	else{
	  	  		Gui_DrawFont_GBK8(0, 36, C_GREEN, C_BLACK, (uint8_t*)"Device is full featured DS18B20");
	  	  		DS18B20_SetConfig(&RomID[0],&Info_backup);  //Restore original setting
	  	  		DS18B20_GetConfig(&RomID[0], &Info_get);  //If OK, show the original setting
	  	  	}
	  	  	sprintf((char *)ShowBuff,"Alarm TL=%3d TH=%3d",Info_get.alarmTL,Info_get.alarmTH);
	  	  	Gui_DrawFont_GBK16(0, 72, C_ORANGE, C_BLACK, ShowBuff);
	  	  	switch (Info_get.resolution)
		  	{
		  		case resolution_9Bit:
		  			sprintf((char *)ShowBuff,"%s",(uint8_t*)"Res= 9bits 0.5'C    ");
		  	  	    break;
		  	  	case resolution_10Bit:
		  	  		sprintf((char *)ShowBuff,"%s",(uint8_t*)"Res= 10bits 0.25'C  ");
		  	  	    break;
		  	  	case resolution_11Bit:
		  	  		sprintf((char *)ShowBuff,"%s",(uint8_t*)"Res= 11bits 0.125'C ");
		  	  	    break;
		  	  	case resolution_12Bit:
		  	  		sprintf((char *)ShowBuff,"%s",(uint8_t*)"Res= 12bits 0.0625'C");
		  	  		break;
		  	  	default:
		  	  		sprintf((char *)ShowBuff,"%s",(uint8_t*)"Res= Data error     ");
		  	  	    break;
		  	}
	  	  	Gui_DrawFont_GBK16(0, 54, C_ORANGE, C_BLACK, ShowBuff);
	  		break;
		}
		case -1:
			Gui_DrawFont_GBK16(0, 36, C_CRIMSON, C_BLACK, (uint8_t*)"No device presence");
			Gui_DrawFont_GBK24(24, 70, C_RED, C_BLACK, (uint8_t*)"==>> EXIT");
			HAL_Delay(2000);
			Lcd_Clear(0, 0, 159, 127, C_BLACK);
			Front_page();
			return DS18B20_ERR_NO_PRESENCE;
		case -3:
			Gui_DrawFont_GBK16(0, 36, C_CRIMSON, C_BLACK, (uint8_t*)"CRC read error    ");
			Gui_DrawFont_GBK24(24, 70, C_RED, C_BLACK, (uint8_t*)"==>> EXIT");
			HAL_Delay(2000);
			Lcd_Clear(0, 0, 159, 127, C_BLACK);
			Front_page();
			return DS18B20_ERR_CRC_VERIFY_FAILED;
		default:
			HAL_Delay(2000);
			Lcd_Clear(0, 0, 159, 127, C_BLACK);
			Front_page();
			return DS18B20_ERR_NO_DEVICES;
	}

	do{
	}while(Read_F1()==0);  //Pause for action
	Lcd_Clear(0, 0, 159, 127, C_BLACK);
	Front_page();
	return DS18B20_NO_ERR;
}

void ds18b20_single_temper(int8_t *_str)
{
	int32_t _Temper_Var;
	DS18B20_ConvertTemperatureAll();
	uint8_t status = DS18b20_single_temper(&_Temper_Var);
	if(status!=0) _Temper_Var = 0;
	int8_t buf[2];
	if(_Temper_Var<0){  //If negative value
		buf[0]=_Temper_Var/10000;
		buf[1]=-_Temper_Var%10000/1000;  // decimal point
	}
	else{
		buf[0]=_Temper_Var/10000;
		buf[1]=_Temper_Var%10000/1000;  // decimal point
	}
	sprintf((char *)_str,"%3d.%d'C",buf[0],buf[1]);
}

void ds18b20_list_temper(void)
{
	Lcd_Clear(0, 0, 159, 127, C_BLACK);
	uint8_t _count = 8;  //Display maximum 8 devices in one page
	uint8_t status = DS18B20_SearchRom(RomID,&_count);
	if(status!=0){
		Gui_DrawFont_GBK24(24, 40, C_RED, C_BLACK, (uint8_t*)"ERROR !!!");
		Gui_DrawFont_GBK24(24, 70, C_RED, C_BLACK, (uint8_t*)"==>> EXIT");
		goto Exit;
	}

	do
	{
		DS18B20_ConvertTemperatureAll();
		HAL_Delay(500);
		for(uint8_t i=0;i<_count;i++){
#if method == 1
			DS18B20_ReadTemperature(&RomID[i], &temper_var);
			Temper_Var = (int32_t)(temper_var*10000);
#else
			DS18B20_ReadTemper(&RomID[i], &Temper_Var);
#endif
				ShowTemper(i);
		}
	}while(Read_F1()==0);  //Exit this loop

	Exit:
	HAL_Delay(2000);
	Lcd_Clear(0, 0, 159, 127, C_BLACK);
	Front_page();
}

void ShowThermometer(void)
{
	Lcd_Clear(0, 0, 159, 127, C_BLACK);
	uint8_t count = 1;  //Set 1 to search for single DS18B20 attached and its code is stored into RomID[0]
	DS18B20_SearchRom(RomID,&count);
//	DS18B20_Read_romcode(&RomID[0]);  //Use this if single DS18B20 attached

	uint16_t temperX = 147;  //Set the X axis position of thermometer
	if(DS18B20_Reset() == 0xFF){  //If no device detected, Set temperature to 0 for display
		Gui_DrawFont_GBK24(10, 90, C_RED, C_BLACK, (uint8_t*)"No DSB1820");
		HAL_Delay(500);
		Lcd_Clear(0, 0, 159, 127, C_BLACK);
	}
	else{
		DS18B20_ConvertTemperatureAll();
		HAL_Delay(200);
#if method == 1
		int8_t status = DS18B20_ReadTemperature(RomID, &temper_var);
		Temper_Var = (int32_t)(temper_var*10000);
#else
		uint8_t status = DS18B20_ReadTemper(&RomID[0], &Temper_Var);
#endif

		if(status == 0){
		Temper_Var = Temper_Var/100;
		DrawGridlines();
		DrawThermometer(temperX);
		}
		else{
			Gui_DrawFont_GBK24(10, 90, C_RED, C_BLACK, (uint8_t*)"Error found");
			HAL_Delay(2000);
			Lcd_Clear(0, 0, 159, 127, C_BLACK);
		}
	}

	do
	{
		if(DS18B20_Reset() == 0xFF){  //If still no device detected, keep display unchanged
			Temper_Var = 0;
			DrawGridlines();
			DrawThermometer(temperX);
			UpdateThermometer(temperX);
		}
	  	else{  //Make calculation and display curve when device attached
	  		DS18B20_ConvertTemperatureAll();
			HAL_Delay(200);
#if method == 1
			int8_t status = DS18B20_ReadTemperature(RomID, &temper_var);
			Temper_Var = (int32_t)(temper_var*10000);
#else
			uint8_t status = DS18B20_ReadTemper(&RomID[0], &Temper_Var);
#endif

			if(status == 0){
				Temper_Var = Temper_Var/100;
				if(Temper_Var >= 0 && Temper_Var <= 9900){  //Only when read temperature value 0'C ~ 99'C
					UpdateThermometer(temperX);
					DrawTemperatureCurve(1);  //Normal curve drawing
				}
				else{
					Lcd_Clear(0, 0, 159, 127, C_BLACK);
					Gui_DrawFont_GBK24(10, 90, C_RED, C_BLACK, (uint8_t*)"Out of Range");
					HAL_Delay(2000);
					Lcd_Clear(0, 0, 159, 127, C_BLACK);
				}
			}
			else{
				Lcd_Clear(0, 0, 159, 127, C_BLACK);
				Gui_DrawFont_GBK24(10, 90, C_RED, C_BLACK, (uint8_t*)"Must restart");
				HAL_Delay(2000);
				Lcd_Clear(0, 0, 159, 127, C_BLACK);
			}
	  	}
	}while(Read_F1()==0);

	DrawTemperatureCurve(0);  //Reset curve drawing
	Temper_Var = 0;  //Initialization for display
	UpdateThermometer(temperX);
	Lcd_Clear(0, 0, 159, 127, C_BLACK);
	Front_page();
}

void DS18B20_function_set(void)
{
	DS18B20_ON();
	LED2_ON();
	Lcd_Clear(0, 0, 159, 127, C_BLACK);
	Front_page();
	do{
		if(Read_F2()){HAL_Delay(200);if(Read_F2()==1) ds18b20_found_rom(8);}
		if(Read_F3()){HAL_Delay(200);if(Read_F3()==1) ds18b20_found_alarm(8);}
		if(Read_F4()){HAL_Delay(200);if(Read_F4()==1) ds18b20_adjust_conf();}
		if(Read_PwrKey()){HAL_Delay(200);if(Read_PwrKey()==1) ds18b20_check();}
		if(Read_L()){HAL_Delay(200);if(Read_L()==1) ds18b20_list_temper();}
		if(Read_R()){HAL_Delay(300);if(Read_R()==1) ShowThermometer();}
	}while(Read_F1()==0);
	Lcd_Clear(0, 0, 159, 127, C_BLACK);
	DS18B20_OFF();
	LED2_OFF();
}
