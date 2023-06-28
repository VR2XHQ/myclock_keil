/****************************************
ST7735 SPI 1.8" TFT driver c file
Ver 1.0

128*160
SPI suggest 16M speed
LCD TFT function
****************************************/
#include "Lcd_Driver.h"
#include "spi.h"

#define SELECT_LCD      HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET)
#define DESELECT_LCD    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET)
#define SEND_COMMAND    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET)
#define SEND_DATA       HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET)
#define LCD_RESET_ON    HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET)
#define LCD_RESET_OFF   HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET)

// front and background color variable
uint16_t FColor;
uint16_t BColor;

/*-----------------------------------------------------------
LCD initialization
--------------------
LCD_RS							PA1
LCD_CS							PA2
LCD_RST							PB8
SPI_SCK							PA5
SPI_MISO						PA6
SPI_MOSI						PA7
------------------------------------------------------------*/
//void LCD_GPIO_Init(void)
//{
	// if using CubeMX, below codes can be ignored
//	GPIO_InitTypeDef GPIO_InitStruct = {0};
//	__HAL_RCC_GPIOA_CLK_ENABLE();
//	__HAL_RCC_GPIOB_CLK_ENABLE();
//
//	/*Configure GPIO pin Output Level */
//	HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
//	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
//	HAL_GPIO_WritePin(GPIOB, LCD_RESET_Pin, GPIO_PIN_RESET);
//
//	/* pin configuration */
//	GPIO_InitStruct.Pin = LCD_RS_Pin;
//	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//	HAL_GPIO_Init(LCD_RS_GPIO_Port, &GPIO_InitStruct);
//
//	/* pin configuration */
//	GPIO_InitStruct.Pin = LCD_CS_Pin;
//	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//	HAL_GPIO_Init(LCD_CS_GPIO_Port, &GPIO_InitStruct);
//
//	/* pin configuration */
//	GPIO_InitStruct.Pin = LCD_RESET_Pin;
//	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//
//	/*SPI initialization */
//	MX_SPI1_Init();  // CubeMX function
//}

// write 8bit command to LCD
void Lcd_WriteIndex(uint8_t Index)
{
	SELECT_LCD;
	SEND_COMMAND;
	HAL_SPI_Transmit(&hspi1, &Index, 1, 1);
	DESELECT_LCD;  
}

// write 8bit data to LCD
void Lcd_WriteData(uint8_t Data)
{
	SELECT_LCD;
	SEND_DATA;
	HAL_SPI_Transmit(&hspi1, &Data, 1, 1);
	DESELECT_LCD; 
}

// write 16bit data to LCD
void LCD_WriteData_16Bit(uint16_t Data)
{
	uint8_t H,L;
	H = Data>>8;
	L = Data;
	SELECT_LCD;
	SEND_DATA;
//	 SPI_WriteData(Data>>8); 	// write high 8bit
//	 SPI_WriteData(Data); 		// write low 8bit
	HAL_SPI_Transmit(&hspi1, &H, 1, 10);
	HAL_SPI_Transmit(&hspi1, &L, 1, 10);
  DESELECT_LCD;  
}

void Lcd_WriteReg(uint8_t Index,uint8_t Data)
{
	Lcd_WriteIndex(Index);
	Lcd_WriteData(Data);
}

void Lcd_Reset(void)
{
	LCD_RESET_OFF;	
	HAL_Delay(1);
	LCD_RESET_ON;   // hardware reset LCD
	HAL_Delay(2);  
	LCD_RESET_OFF;
	HAL_Delay(10);
}

//LCD Init For 1.44Inch LCD Panel with ST7735R.
void Lcd_Init(void)
{	
//	LCD_GPIO_Init();	// LCD initialization
	Lcd_Reset();		// LCD hardware reset

	#ifdef ST7735B
 //-----LCD Software Reset------//  
    Lcd_WriteIndex(0x01);   //Software setting
    HAL_Delay(10);
    Lcd_WriteIndex(0x11);   //sleep out
    HAL_Delay(1);

    Lcd_WriteIndex(0x3A);   // RGB color  format  select
    Lcd_WriteData(0x05);  // 03: 12bit  05: 16bit  06: 18bit
    //HAL_Delay(1);

    Lcd_WriteIndex(0xB1);  // Frame rate setting   95hz
    Lcd_WriteData(0x00);
    Lcd_WriteData(0x06);
    Lcd_WriteData(0x03);
    //HAL_Delay(1);

    Lcd_WriteIndex(0x36);  // MV,MX,MY 
#if LCD_XY_Mode == 0 
	Lcd_WriteData(0x48); // horizontal screen set 48, C8 flip screen
#else
    Lcd_WriteData(0x68); // vertical screen set 68, A8 flip screen
#endif   	

    Lcd_WriteIndex(0xB6);  	// Display control non-overlap
    Lcd_WriteData(0x15); 
    Lcd_WriteData(0x02);  

    Lcd_WriteIndex(0xB4);  	// line inversion
    Lcd_WriteData(0x00); 

    //POWER ON SETTING 
    Lcd_WriteIndex(0xC0);	//VCI1=2.65V
    Lcd_WriteData(0x02);
    Lcd_WriteData(0x70);
    HAL_Delay(1);
    Lcd_WriteIndex(0xC1);   
    Lcd_WriteData(0X05);  	// VGH,VGL
    Lcd_WriteIndex(0xC2);  
    Lcd_WriteData(0X01);
    Lcd_WriteData(0X02);   	// DC/DC SET
    Lcd_WriteIndex(0xC5); 	// VCOMH=3.925V;VCOML=0.875V,
    Lcd_WriteData(0x3C);  
    Lcd_WriteData(0x38);
    //HAL_Delay(1);

    Lcd_WriteIndex(0xFC); 
    Lcd_WriteData(0x11);
    Lcd_WriteData(0x15);
    //GAMMA SETTING
    
    Lcd_WriteIndex(0xE0);// Positive
    Lcd_WriteData(0x09);//+1
    Lcd_WriteData(0x16);//+2
    Lcd_WriteData(0x09);//+3
    Lcd_WriteData(0x20);//+4
    Lcd_WriteData(0x21);//+5
    Lcd_WriteData(0x1B);//+6
    Lcd_WriteData(0x13);//+7
    Lcd_WriteData(0x19);//+8
    Lcd_WriteData(0x17);//+9
    Lcd_WriteData(0x15);//+10
    Lcd_WriteData(0x1E);//+11
    Lcd_WriteData(0x2B);//+12
    Lcd_WriteData(0x04);//+13
    Lcd_WriteData(0x05);//+14
    Lcd_WriteData(0x02);//+15
    Lcd_WriteData(0x0E);//+16

    Lcd_WriteIndex(0xE1);// Negative
    Lcd_WriteData(0x0B);//-1
    Lcd_WriteData(0x14);//-2
    Lcd_WriteData(0x08);//-3
    Lcd_WriteData(0x1E);//-4
    Lcd_WriteData(0x22);//-5
    Lcd_WriteData(0x1D);//-6
    Lcd_WriteData(0x18);//-7
    Lcd_WriteData(0x1E);//-8
    Lcd_WriteData(0x1B);//-9
    Lcd_WriteData(0x1A);//-10
    Lcd_WriteData(0x24);//-11
    Lcd_WriteData(0x2B);//-12
    Lcd_WriteData(0x06);//-13
    Lcd_WriteData(0x06);//-14
    Lcd_WriteData(0x02);//-15
    Lcd_WriteData(0x0F);//-16
    //HAL_Delay(1);

    //window setting
    Lcd_WriteIndex(0x2A);
    Lcd_WriteData(0x00);
    Lcd_WriteData(LCD_XS);
    Lcd_WriteData(0x00);
    Lcd_WriteData(LCD_XE);
  
    Lcd_WriteIndex(0x2B);
    Lcd_WriteData(0x00);
    Lcd_WriteData(LCD_YS);
    Lcd_WriteData(0x00);
    Lcd_WriteData(LCD_YE);

    //Lcd_WriteIndex(0x26);  // default gamma setting 
    //Lcd_WriteIndex(0x30);  // partial area setting 
   
    Lcd_WriteIndex(0x13);
    //HAL_Delay(1);
    
    Lcd_WriteIndex(0x2C);
    HAL_Delay(1);

    Lcd_WriteIndex(0x29);  //display  on setting 
    HAL_Delay(1);
	#else
	/*ST7735R Drive*/
		//LCD Init For 1.44Inch LCD Panel with ST7735R. ??? 1.8"
		Lcd_WriteIndex(0x11);	//Sleep exit
		HAL_Delay(120);
			
		//ST7735R Frame Rate
		Lcd_WriteIndex(0xB1); 
		Lcd_WriteData(0x01); 
		Lcd_WriteData(0x2C); 
		Lcd_WriteData(0x2D); 

		Lcd_WriteIndex(0xB2); 
		Lcd_WriteData(0x01); 
		Lcd_WriteData(0x2C); 
		Lcd_WriteData(0x2D); 

		Lcd_WriteIndex(0xB3); 
		Lcd_WriteData(0x01); 
		Lcd_WriteData(0x2C); 
		Lcd_WriteData(0x2D); 
		Lcd_WriteData(0x01); 
		Lcd_WriteData(0x2C); 
		Lcd_WriteData(0x2D); 

		Lcd_WriteIndex(0xB4); //Column inversion 
		Lcd_WriteData(0x07); 

		//ST7735R Power Sequence
		Lcd_WriteIndex(0xC0); 
		Lcd_WriteData(0xA2); 
		Lcd_WriteData(0x02); 
		Lcd_WriteData(0x84); 
		Lcd_WriteIndex(0xC1); 
		Lcd_WriteData(0xC5); 

		Lcd_WriteIndex(0xC2); 
		Lcd_WriteData(0x0A); 
		Lcd_WriteData(0x00); 

		Lcd_WriteIndex(0xC3); 
		Lcd_WriteData(0x8A); 
		Lcd_WriteData(0x2A); 
		Lcd_WriteIndex(0xC4); 
		Lcd_WriteData(0x8A); 
		Lcd_WriteData(0xEE); 

		Lcd_WriteIndex(0xC5); //VCOM 
		Lcd_WriteData(0x0E);

		Lcd_WriteIndex(0x36); //MX, MY, RGB mode 
//		Lcd_WriteData(0xC0); ???
		#if LCD_XY_Mode == 0 
			Lcd_WriteData(0x48); // horizontal screen set 48, C8 flip screen
		#else
				Lcd_WriteData(0x60); // vertical screen set 68, A8 flip screen
		#endif   

		//ST7735R Gamma Sequence, notes: disable seems better
//		Lcd_WriteIndex(0xe0); 
//		Lcd_WriteData(0x0f); 
//		Lcd_WriteData(0x1a); 
//		Lcd_WriteData(0x0f); 
//		Lcd_WriteData(0x18); 
//		Lcd_WriteData(0x2f); 
//		Lcd_WriteData(0x28); 
//		Lcd_WriteData(0x20); 
//		Lcd_WriteData(0x22); 
//		Lcd_WriteData(0x1f); 
//		Lcd_WriteData(0x1b); 
//		Lcd_WriteData(0x23); 
//		Lcd_WriteData(0x37); 
//		Lcd_WriteData(0x00); 	
//		Lcd_WriteData(0x07); 
//		Lcd_WriteData(0x02); 
//		Lcd_WriteData(0x10); 

//		Lcd_WriteIndex(0xe1); 
//		Lcd_WriteData(0x0f); 
//		Lcd_WriteData(0x1b); 
//		Lcd_WriteData(0x0f); 
//		Lcd_WriteData(0x17); 
//		Lcd_WriteData(0x33); 
//		Lcd_WriteData(0x2c); 
//		Lcd_WriteData(0x29); 
//		Lcd_WriteData(0x2e); 
//		Lcd_WriteData(0x30); 
//		Lcd_WriteData(0x30); 
//		Lcd_WriteData(0x39); 
//		Lcd_WriteData(0x3f); 
//		Lcd_WriteData(0x00); 
//		Lcd_WriteData(0x07); 
//		Lcd_WriteData(0x03); 
//		Lcd_WriteData(0x10);  

		Lcd_WriteIndex(0x2A);		// set DRAM display area
		Lcd_WriteData(0x00);
		Lcd_WriteData(0x01);		//0x01
		Lcd_WriteData(0x00);
		Lcd_WriteData(0xA0);		//0xA0

		Lcd_WriteIndex(0x2B);
		Lcd_WriteData(0x00);
		Lcd_WriteData(0x02);		//0x02
		Lcd_WriteData(0x00);
		Lcd_WriteData(0x81);		//0x81

		Lcd_WriteIndex(0xF0); //Enable test command  
		Lcd_WriteData(0x01); 
		Lcd_WriteIndex(0xF6); //Disable ram power save mode 
		Lcd_WriteData(0x00); 

		Lcd_WriteIndex(0x3A); //65k mode 
		Lcd_WriteData(0x05); 

		Lcd_WriteIndex(0x29);//Display on

  #endif
//	Lcd_WriteIndex(0x2A);			// set DRAM display area
//	Lcd_WriteData(0x00);
//	Lcd_WriteData(0x01);
//	Lcd_WriteData(0x00);
//	Lcd_WriteData(0xA0);

//	Lcd_WriteIndex(0x2B);
//	Lcd_WriteData(0x00);
//	Lcd_WriteData(0x02);
//	Lcd_WriteData(0x00);
//	Lcd_WriteData(0x81);
//	
//	uint16_t i;  	
//	Lcd_WriteIndex(0x2C);
//  for(i=0;i<20480;i++)
//	{
//		LCD_DataWrite(hei);		
//	}
/*	******************************
	LCD_GPIO_Init();
	Lcd_Reset(); //Reset before LCD Init.

	//LCD Init For 1.44Inch LCD Panel with ST7735R.
	Lcd_WriteIndex(0x11);//Sleep exit 
	HAL_Delay(120);
		
	//ST7735R Frame Rate
	Lcd_WriteIndex(0xB1); 
	Lcd_WriteData(0x01); 
	Lcd_WriteData(0x2C); 
	Lcd_WriteData(0x2D); 

	Lcd_WriteIndex(0xB2); 
	Lcd_WriteData(0x01); 
	Lcd_WriteData(0x2C); 
	Lcd_WriteData(0x2D); 

	Lcd_WriteIndex(0xB3); 
	Lcd_WriteData(0x01); 
	Lcd_WriteData(0x2C); 
	Lcd_WriteData(0x2D); 
	Lcd_WriteData(0x01); 
	Lcd_WriteData(0x2C); 
	Lcd_WriteData(0x2D); 
	
	Lcd_WriteIndex(0xB4); //Column inversion 
	Lcd_WriteData(0x07); 
	
	//ST7735R Power Sequence
	Lcd_WriteIndex(0xC0); 
	Lcd_WriteData(0xA2); 
	Lcd_WriteData(0x02); 
	Lcd_WriteData(0x84); 
	Lcd_WriteIndex(0xC1); 
	Lcd_WriteData(0xC5); 

	Lcd_WriteIndex(0xC2); 
	Lcd_WriteData(0x0A); 
	Lcd_WriteData(0x00); 

	Lcd_WriteIndex(0xC3); 
	Lcd_WriteData(0x8A); 
	Lcd_WriteData(0x2A); 
	Lcd_WriteIndex(0xC4); 
	Lcd_WriteData(0x8A); 
	Lcd_WriteData(0xEE); 
	
	Lcd_WriteIndex(0xC5); //VCOM 
	Lcd_WriteData(0x0E); 
	
	Lcd_WriteIndex(0x36); //MX, MY, RGB mode 
//	Lcd_WriteData(0xA0);
	Lcd_WriteData(0x60);
	
	//ST7735R Gamma Sequence
	Lcd_WriteIndex(0xe0); 
	Lcd_WriteData(0x0f); 
	Lcd_WriteData(0x1a); 
	Lcd_WriteData(0x0f); 
	Lcd_WriteData(0x18); 
	Lcd_WriteData(0x2f); 
	Lcd_WriteData(0x28); 
	Lcd_WriteData(0x20); 
	Lcd_WriteData(0x22); 
	Lcd_WriteData(0x1f); 
	Lcd_WriteData(0x1b); 
	Lcd_WriteData(0x23); 
	Lcd_WriteData(0x37); 
	Lcd_WriteData(0x00); 	
	Lcd_WriteData(0x07); 
	Lcd_WriteData(0x02); 
	Lcd_WriteData(0x10); 

	Lcd_WriteIndex(0xe1); 
	Lcd_WriteData(0x0f); 
	Lcd_WriteData(0x1b); 
	Lcd_WriteData(0x0f); 
	Lcd_WriteData(0x17); 
	Lcd_WriteData(0x33); 
	Lcd_WriteData(0x2c); 
	Lcd_WriteData(0x29); 
	Lcd_WriteData(0x2e); 
	Lcd_WriteData(0x30); 
	Lcd_WriteData(0x30); 
	Lcd_WriteData(0x39); 
	Lcd_WriteData(0x3f); 
	Lcd_WriteData(0x00); 
	Lcd_WriteData(0x07); 
	Lcd_WriteData(0x03); 
	Lcd_WriteData(0x10);  
	
	Lcd_WriteIndex(0x2a);
	Lcd_WriteData(0x00);
	Lcd_WriteData(0x01);
	Lcd_WriteData(0x00);
	Lcd_WriteData(0xA0);

	Lcd_WriteIndex(0x2b);
	Lcd_WriteData(0x00);
	Lcd_WriteData(0x02);
	Lcd_WriteData(0x00);
	Lcd_WriteData(0x81);
	
	Lcd_WriteIndex(0xF0); //Enable test command  
	Lcd_WriteData(0x01); 
	Lcd_WriteIndex(0xF6); //Disable ram power save mode 
	Lcd_WriteData(0x00); 
	
	Lcd_WriteIndex(0x3A); //65k mode 
	Lcd_WriteData(0x05); 
	
	
	Lcd_WriteIndex(0x29);//Display on	 
	*/
}


/*************************************************
name: LCD_Set_Region
function: set lcd display area, auto line feed
parameter: xy start and end coordinates
return: null
*************************************************/
void Lcd_SetRegion(uint16_t x_start,uint16_t y_start,uint16_t x_end,uint16_t y_end)
{		
	Lcd_WriteIndex(0x2a);
//	Lcd_WriteData(0x02);
	Lcd_WriteData(0x00);
	Lcd_WriteData(x_start+LCD_XS);
	Lcd_WriteData(0x00);
	Lcd_WriteData(x_end+LCD_XS);

	Lcd_WriteIndex(0x2b);
//	Lcd_WriteData(0x02);
	Lcd_WriteData(0x00);
	Lcd_WriteData(y_start+LCD_YS);
	Lcd_WriteData(0x00);
	Lcd_WriteData(y_end+LCD_YS);
	
	Lcd_WriteIndex(0x2c);
}

/*************************************************
name: LCD_SetXY
function: set lcd display start point
parameter: xy coordinates
return: null
*************************************************/
void Lcd_SetXY(uint16_t x,uint16_t y)
{
  	Lcd_SetRegion(x,y,x,y);
}
	
/*************************************************
name: Gui_DrawPoint
function: draw a point
parameter: xy coordinates
return: null
*************************************************/
void Gui_DrawPoint(uint16_t x,uint16_t y,uint16_t Data)
{
	Lcd_SetRegion(x,y,x+1,y+1);
	LCD_WriteData_16Bit(Data);
}    

/*****************************************
function: read TFT color value at xy
out parameter: color value
******************************************/
//unsigned int Lcd_ReadPoint(uint16_t x,uint16_t y)
//{
//  unsigned int Data;
//  Lcd_SetXY(x,y);
//
////  Lcd_ReadData(); // drop no use byte ??
////  Data=Lcd_ReadData();
//  Lcd_WriteData(Data);
//  return Data;
//}

/*************************************************
name: Gui_DrawLine
function: draw line by Bresenham method
parameter: xy coordinates
return: null
*************************************************/
void Gui_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t Color)
{
int dx,             // difference in x's
    dy,             // difference in y's
    dx2,            // dx,dy * 2
    dy2, 
    x_inc,          // amount in pixel space to move during drawing
    y_inc,          // amount in pixel space to move during drawing
    error,          // the discriminant i.e. error i.e. decision variable
    index;          // used for looping	

	Lcd_SetXY(x0,y0);
	dx = x1-x0; // calculate x distance
	dy = y1-y0; // calculate y distance

	if (dx>=0)
	{
		x_inc = 1;
	}
	else
	{
		x_inc = -1;
		dx    = -dx;  
	} 
	
	if (dy>=0)
	{
		y_inc = 1;
	} 
	else
	{
		y_inc = -1;
		dy    = -dy; 
	} 

	dx2 = dx << 1;
	dy2 = dy << 1;

	if (dx > dy)
	{
		// initialize error term
		error = dy2 - dx; 

		// draw the line
		for (index=0; index <= dx; index++)
		{
			Gui_DrawPoint(x0,y0,Color);
			
			// test if error has overflowed
			if (error >= 0)
			{
				error-=dx2;

				// move to next line
				y0+=y_inc;
			} // end if error overflowed

			// adjust the error term
			error+=dy2;

			// move to the next pixel
			x0+=x_inc;
		} // end for
	} // end if |slope| <= 1
	else
	{
		// initialize error term
		error = dx2 - dy; 

		// draw the line
		for (index=0; index <= dy; index++)
		{
			// set the pixel
			Gui_DrawPoint(x0,y0,Color);

			// test if error overflowed
			if (error >= 0)
			{
				error-=dy2;

				// move to next line
				x0+=x_inc;
			} // end if error overflowed

			// adjust the error term
			error+=dx2;

			// move to the next pixel
			y0+=y_inc;
		} // end for
	} // end else |slope| > 1
}

void Gui_DrawFill(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t Color)
{
	Lcd_Clear(xs, ys, xe, ye, Color);
}

/*************************************************
name: Lcd_Clear
function: clear display of selected area
parameter: xy coordinates, filled color
return: null
*************************************************/
void Lcd_Clear(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t Color)               
{	
  unsigned int i,m;
	if((xe >= xs) && (ye >= ys))
	{
		Lcd_SetRegion(xs,ys,xe,ye);
		for(i=ys; i<=ye; i++)
		for(m=xs; m<=xe; m++)
		{	
			LCD_WriteData_16Bit(Color);
		}
	}		
}
