/*
 * Thermometer.c
 *
 *  Created on: Oct 10, 2021
 *  Last edited on: Nov 17, 2022
 *      Author: Ben Yuen
 */

#include "Thermometer.h"
#include "ds18b20.h"
#include "GUI.h"
#include "Lcd_Driver.h"
#include "SD3078_rtc.h"
#include <stdio.h>

TemperatureList templist[120]={0};  //Define array structure
__IO uint8_t templist_index = 0;  //Initial value for the index of templist
int16_t temper_Coeff;  //Temperature coefficient
uint8_t	showbuff[48];  //Buff for content display

/***************************************
Function: Draw temperature bar
Parameter In: temperature bar X axis, value 9-151
Parameter Out: None
***************************************/
void DrawThermometer(uint16_t dx)
{
	uint16_t c;  //Set the scale color
	Gui_RoundRect(dx-2,1,dx+2,118,2,C_DARK_GRAY);  //Draw the outline of thermometer
	Gui_FillCircle(dx,121,6,C_DARK_GRAY);  //Draw the outline of bottom circle outline
	Gui_FillCircle(dx,121,5,C_RED);  //Draw the inner of bottom circle
	Gui_RoundRect(dx-1,2,dx+1,103,2,C_BLACK);  //Draw the inside upper part of thermometer
	Gui_RoundRect(dx-1,102,dx+1,122,2,C_RED);  //Draw the inside lower part of thermometer

	for(uint8_t i=2; i<117; i+=10)  //Draw the scale
	{
		if(i>=102)
			c = C_GREEN;
		else
			c = C_FIREBRICK;

		if(i%20 == 2)
		{
			Gui_DrawLine(dx-7,i,dx-4,i,c);
			Gui_DrawLine(dx+7,i,dx+4,i,c);
		}
		else
		{
			Gui_DrawLine(dx-5,i,dx-4,i,c);
			Gui_DrawLine(dx+5,i,dx+4,i,c);
		}
	}
}

/***************************************
Function: Update thermometer bar and temperature display
Parameter In: temperature bar X axis
Parameter Out: None
***************************************/
void UpdateThermometer(uint16_t dx)
{
	static uint16_t temperY;  //Temperature bar Y axis value
	static uint16_t temper_old;  //Last time temperature

	/* Update the temperature bar */
	temperY = 102-(Temper_Var/100);  //Y axis 102 as 0'C
	if(Temper_Var > temper_old)
	{
		Gui_RoundRect(dx-1,temperY,dx+1,102,2,C_RED);  //Draw red bar
	}
	else if(Temper_Var < temper_old)
	{
		Gui_RoundRect(dx-1,2,dx+1,temperY,2,C_BLACK);  //Draw black bar
	}

	temper_old = Temper_Var;  //Set Current value as Last temperature for next cycle

	uint8_t data[6];  //Show temperature in digital font
	data[0]=Temper_Var/1000;
	data[1]=Temper_Var%1000/100;
	data[2]=0xA;  // decimal point
	data[3]=Temper_Var%100/10;
	data[4]=Temper_Var%10;
	data[5]=0x10;  //EOString
	Gui_DrawFont_Num32Str(2, 90, C_LIME, C_BLACK, data);
}

/***************************************
Function: Draw grid for temperature curve
Parameter In: None
Parameter Out: None
***************************************/
void DrawGridlines(void)
{
	/* Draw grid */
	Gui_DrawLine(8,0,129,0,C_DIM_GRAY);
	Gui_DrawLine(8,0,8,81,C_DIM_GRAY);
	Gui_DrawLine(129,0,129,81,C_DIM_GRAY);
	Gui_DrawLine(8,81,129,81,C_DIM_GRAY);
	Gui_DrawLine(9,40,128,40,C_DIM_GRAY);
	Gui_DrawLine(68,1,68,80,C_DIM_GRAY);

	uint8_t i;
	for(i=9; i<129; i+=5)  //Draw dots line
	{
		Gui_DrawPoint(i, 41, C_WHITE);
	}
	for(i=1; i<81; i+=5)
	{
		Gui_DrawPoint(69, i, C_WHITE);
	}
}

/***************************************
Function: Draw temperature curve
Parameter In: None
Parameter Out: None
***************************************/
void DrawTemperatureCurve(uint8_t on)
{
	static uint8_t old_period;
	static int16_t y0, y;
	static uint16_t x_old,y_old;
	static int16_t t;
	uint16_t m,n;

	if(on==0)  //Initial for new cycle
	{
		templist_index=0;
	}
	else
	{
		ReadTime(&Time);  //Read time

	#if (update_time==sec)  //Update per second
			if(old_period != Time.second)
			{
				old_period = Time.second;
//				templist[templist_index].period = Time.second;  // not use

	#elif (update_time==min)  //Update per minute
			if(old_period != Time.minute)
			{
				old_period = Time.minute;
//				templist[templist_index].period = Time.minute;  //not use
	#endif

			templist[templist_index].temper = Temper_Var;
			t = (templist[templist_index].temper + 50)/10;  //Round up figure of temperature get currently

			if(templist_index == 0)
			{
				y0 = (templist[0].temper + 50)/10;  //Round up figure, beginning temperature as reference
				temper_Coeff = (y0+50)/50;  //Initialize the coefficient for scale range
				m = (float)y0 + 40.0f/(float)temper_Coeff;  //Value of upper limit
				sprintf((char *)showbuff, "%1d%1d.%1d", m/100, m%100/10, m%10);
				Gui_DrawFont_GBK8(10, 2, C_SNOW, C_RED, showbuff);
				n = (float)y0 - 50.0f/(float)temper_Coeff;  //Value of lower limit
				sprintf((char *)showbuff, "%1d%1d.%1d", n/100, n%100/10, n%10);
				Gui_DrawFont_GBK8(10, 71, C_SNOW, C_BLUE, showbuff);
			}

			y = 41 - (t - y0)*temper_Coeff;  //Calculate the different between current and initial temperature
			if(y<1) y=1;
			if(y>80) y=80;
			if(y<2 || y>79)  //Y axis limit reach then adjust coefficient
			{
				if(temper_Coeff>1) temper_Coeff--;  //Adjust only when coefficient larger than 1
				y = 41 - (t - y0)*temper_Coeff;
				if(y<1) y=1;
				if(y>80) y=80;
				Gui_DrawFill(8,0,129,81, C_BLACK);  //Redraw according to new coefficient
				DrawGridlines();
				m = (float)y0 + 40.0f/(float)temper_Coeff;  //Value of upper limit
				sprintf((char *)showbuff, "%1d%1d.%1d", m/100, m%100/10, m%10);
				Gui_DrawFont_GBK8(10, 2, C_SNOW, C_RED, showbuff);
				n = (float)y0 - 50.0f/(float)temper_Coeff;  //Value of lower limit
				sprintf((char *)showbuff, "%1d%1d.%1d", n/100, n%100/10, n%10);
				Gui_DrawFont_GBK8(10, 71, C_SNOW, C_BLUE, showbuff);
				for(uint16_t i=0; i < templist_index; i++)  //Redraw whole curve up to index position
				{
					if(i == 0)  //Process the beginning of the curve
					{
						Gui_DrawPoint(i+9, 41, C_CYAN);
						x_old = i+9;
						y_old = 41;
					}
					else  //Set the other points up according to temperature record in templist
					{
						Gui_DrawLine(x_old, y_old, i+9, 41-((templist[i].temper+50)/10-y0)*temper_Coeff, C_CYAN);
						x_old = i+9;
						y_old = 41-((templist[i].temper+50)/10-y0)*temper_Coeff;
					}
				}
			}

			if(templist_index == 0)  //Set the beginning point of the curve
			{
				Gui_DrawPoint(templist_index+9, 41, C_CYAN);
				x_old = templist_index+9;
				y_old = 41;
			}
			else  //Set the other points according to each time slot
			{
				Gui_DrawLine(x_old, y_old, templist_index+9, (uint16_t)y, C_CYAN);
				x_old = templist_index+9;
				y_old = y;
			}

			templist_index++;
			if(templist_index > 119)  //Redraw and reset index for next cycle when reach 120 minutes
			{
				templist_index = 0;
				Gui_DrawFill(8,0,129,81, C_BLACK);
				DrawGridlines();
			}
		}
	}
}
