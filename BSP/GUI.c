/****************************************
ST7735 SPI 1.8" TFT driver c file
Ver 1.0

128*160
SPI suggest 16M speed
LCD TFT function
****************************************/
#include "GUI.h"
#include "font_ext.h"
#include "stdio.h"

//struct SkinType Skin_Current;
//
//struct SkinType Skin[4] = {
//	{
//		.skin_name 			= "�����",
//		.main_fc				= C_BEIGE,
//		.main_bc				= 0x1082,
//		.statusup_fc		= C_DEEP_PINK,
//		.statusup_bc		= 0x2104,
//		.statusdown_fc	= C_SNOW,
//		.statusdown_bc	= 0x4808,
//		.lab_fc					= C_DEEP_SKY_BLUE,
//		.lab_dfc				= 0x2104,
//		.lab_bc					= C_BLACK,
//		.menubar_fc			= C_SNOW,
//		.menubar_bc			= 0x4808,
//		.menu_fc				= C_SNOW,
//		.menu_bc				= 0x1082
//	},
//
//	{
//		.skin_name 			= "�����",
//		.main_fc				= C_BEIGE,
//		.main_bc				= 0x1082,
//		.statusup_fc		= C_DEEP_PINK,
//		.statusup_bc		= 0x2104,
//		.statusdown_fc	= C_SNOW,
//		.statusdown_bc	= 0x4808,
//		.lab_fc					= C_DEEP_SKY_BLUE,
//		.lab_dfc				= 0x2104,
//		.lab_bc					= C_BLACK,
//		.menubar_fc			= C_SNOW,
//		.menubar_bc			= 0x4808,
//		.menu_fc				= C_SNOW,
//		.menu_bc				= 0x1082
//	},
//
//	{
//		.skin_name 			= "�����",
//		.main_fc				= C_BEIGE,
//		.main_bc				= 0x1082,
//		.statusup_fc		= C_DEEP_PINK,
//		.statusup_bc		= 0x2104,
//		.statusdown_fc	= C_SNOW,
//		.statusdown_bc	= 0x4808,
//		.lab_fc					= C_DEEP_SKY_BLUE,
//		.lab_dfc				= 0x2104,
//		.lab_bc					= C_BLACK,
//		.menubar_fc			= C_SNOW,
//		.menubar_bc			= 0x4808,
//		.menu_fc				= C_SNOW,
//		.menu_bc				= 0x1082
//	},
//
//	{
//		.skin_name 			= "�����",
//		.main_fc				= C_BEIGE,
//		.main_bc				= 0x1082,
//		.statusup_fc		= C_DEEP_PINK,
//		.statusup_bc		= 0x2104,
//		.statusdown_fc	= C_SNOW,
//		.statusdown_bc	= 0x4808,
//		.lab_fc					= C_DEEP_SKY_BLUE,
//		.lab_dfc				= 0x2104,
//		.lab_bc					= C_BLACK,
//		.menubar_fc			= C_SNOW,
//		.menubar_bc			= 0x4808,
//		.menu_fc				= C_SNOW,
//		.menu_bc				= 0x1082
//	}
//};
//
//
//

uint16_t LCD_BGR2RGB(uint16_t c);
void Gui_Circle(uint16_t X,uint16_t Y,uint16_t R,uint16_t fc) ;


//��ILI93xx����������ΪGBR��ʽ��������д���ʱ��ΪRGB��ʽ��
//ͨ���ú���ת��
//c:GBR��ʽ����ɫֵ
//����ֵ��RGB��ʽ����ɫֵ
uint16_t LCD_BGR2RGB(uint16_t c)
{
  uint16_t  r,g,b,rgb;   
  b=(c>>0)&0x1f;
  g=(c>>5)&0x3f;
  r=(c>>11)&0x1f;	 
  rgb=(b<<11)+(g<<5)+(r<<0);		 
  return(rgb);

}


//��Բ����  ��
void Gui_Circle(uint16_t X,uint16_t Y,uint16_t R,uint16_t fc) 
{//Bresenham�㷨 
    unsigned short  a,b; 
    int c; 
    a=0; 
    b=R; 
    c=3-2*R; 
    while (a<b) 
    { 
        Gui_DrawPoint(X+a,Y+b,fc);     //        7 
        Gui_DrawPoint(X-a,Y+b,fc);     //        6 
        Gui_DrawPoint(X+a,Y-b,fc);     //        2 
        Gui_DrawPoint(X-a,Y-b,fc);     //        3 
        Gui_DrawPoint(X+b,Y+a,fc);     //        8 
        Gui_DrawPoint(X-b,Y+a,fc);     //        5 
        Gui_DrawPoint(X+b,Y-a,fc);     //        1 
        Gui_DrawPoint(X-b,Y-a,fc);     //        4 
				
        if(c<0) c=c+4*a+6; 
        else 
        { 
            c=c+4*(a-b)+10; 
            b-=1; 
        } 
       a+=1; 
    } 
    if (a==b) 
    { 
        Gui_DrawPoint(X+a,Y+b,fc); 
        Gui_DrawPoint(X+a,Y+b,fc); 
        Gui_DrawPoint(X+a,Y-b,fc); 
        Gui_DrawPoint(X-a,Y-b,fc); 
        Gui_DrawPoint(X+b,Y+a,fc); 
        Gui_DrawPoint(X-b,Y+a,fc); 
        Gui_DrawPoint(X+b,Y-a,fc); 
        Gui_DrawPoint(X-b,Y-a,fc); 
			
    } 
} 



/**************************************************************************************
��������: ����Ļָ�����괦��ʾԲ�Ǿ���
��    ��: uint16_t xs,ys, xe,ye �ַ���ʼ�������� bc����ɫ
��    ��: ��
**************************************************************************************/
void Gui_RoundRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t r, uint16_t fc)
{
	int16_t  x,y,xd;

	if ( x2 < x1 )
	{
		x = x2;
		x2 = x1;
		x1 = x;
	}
	if ( y2 < y1 )
	{
		y = y2;
		y2 = y1;
		y1 = y;
	}

	if ( r<=0 ) return;

	xd = 3 - (r << 1);
	x = 0;
	y = r;

	Gui_DrawFill(x1 + r, y1, x2 - r, y2, fc);

	while ( x <= y )
	{
		if( y > 0 )
		{
			Gui_DrawLine(x2 + x - r, y1 - y + r, x2+ x - r, y + y2 - r, fc);
			Gui_DrawLine(x1 - x + r, y1 - y + r, x1- x + r, y + y2 - r, fc);
		}
		if( x > 0 )
		{
			Gui_DrawLine(x1 - y + r, y1 - x + r, x1 - y + r, x + y2 - r, fc);
			Gui_DrawLine(x2 + y - r, y1 - x + r, x2 + y - r, x + y2 - r, fc);
		}
		if ( xd < 0 )
		{
			xd += (x << 2) + 6;
		}
		else
		{
			xd += ((x - y) << 2) + 10;
			y--;
		}
		x++;
	}
}



/**************************************************************************************
��������: ����Ļָ�����괦��ʾʵ��Բ
��    ��: uint16_t xs,ys, fcǰ��ɫ
��    ��: ��
**************************************************************************************/
void Gui_FillCircle(uint16_t x0,uint16_t y0,uint16_t r,uint16_t fc)
{											  
	uint32_t i;
	uint32_t imax = ((uint32_t)r*707)/1000+1;
	uint32_t sqmax = (uint32_t)r*(uint32_t)r+(uint32_t)r/2;
	uint32_t x=r;
	Gui_DrawLine(x0-r,y0,(x0-r)+2*r,y0,fc);
	for (i=1;i<=imax;i++) 
	{
		if ((i*i+x*x)>sqmax) 
		{
			// draw lines from outside  
			if (x>imax) 
			{
				Gui_DrawLine (x0-i+1,y0+x,(x0-i+1)+2*(i-1),y0+x,fc);
				Gui_DrawLine (x0-i+1,y0-x,(x0-i+1)+2*(i-1),y0-x,fc);
			}
			x--;
		}
		// draw lines from inside (center)  
		Gui_DrawLine(x0-x,y0+i,(x0-x)+2*x,y0+i,fc);
		Gui_DrawLine(x0-x,y0-i,(x0-x)+2*x,y0-i,fc);
	}
}


/**************************************************************************************
��������: ����Ļָ�����괦��ʾ�ָ�8�ĵ���Ӣ���ַ�
��    ��: uint16_t x,y �ַ���ʼ�������� fc����ɫ bc����ɫ
��    ��: ��
**************************************************************************************/
void Gui_DrawFont_GBK8_Char(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t c)
{
 uint8_t i,j;
 unsigned int k;
 if ((c<32) || (c>126))	   		// �жϲ����Ƿ���Ӣ���ַ�
 	return;
 
 c -= 0x20;	   				   // �����ַ����ֿ��е�λ��
 k = ((unsigned int)c)<<3; 	   // ȡ�ַ����ֿ��е���ʼλ��
 
 Lcd_SetRegion(x,y,x+4,y+7);			//����λ�����ʾ����ʾ�����Ż�
 for(i=0;i<8;i++)			   // ѭ����ʾ��1���ַ�8�ֽ�����
 {
  for(j=0;j<5;j++)			   // ȡÿ�ֽڵĸ�5λ
  {
   if (asc8[k+i] & (0x80>>j))	// ������ģ����1�����ص���������ɫ���
		 LCD_WriteData_16Bit(fc);
//	   Gui_DrawPoint(x+j,y+i,fc);
   else
	   if (fc!=bc)
			 LCD_WriteData_16Bit(bc);
//			 Gui_DrawPoint(x+j,y+i,bc);	// ������ģ����0�����ص��ñ�����ɫ���
  }// for2 end
 }// for1 end 
 
}


/**************************************************************************************
��������: ����Ļָ�����괦��ʾ�ָ�8��Ӣ���ַ���
��    ��: uint16_t x, y �ַ�����ʼ�������� fc����ɫ bc����ɫ
��    ��: ��
**************************************************************************************/
void Gui_DrawFont_GBK8(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *str)
{ 

 while(*str!='\0')
 {
  if (*str > 127 ) 	  	// �ж��������ַ�ֱ�ӷ��ز���ʾ
		return;
  else	   	 	   	  	// ��ʾһ��Ӣ���ַ�
  {
		if((x+4)>LCD_XE) 	  // x����Խ����
		{
		 x = 0;
		 y += 8;
		}

		if((y+7)>LCD_YE)	  // y����Խ����
		{
		 y = 0; 
		}  
  
	Gui_DrawFont_GBK8_Char(x, y, fc, bc, *str);		// LCD����ʾһ���ַ�
	str++;
	x += 5;	  				// x�����Ƶ���һ���ַ�λ�� 
	}
 }// while end
}


/**************************************************************************************
��������: ����Ļָ�����괦��ʾ�ָ�16�ĵ���Ӣ���ַ�
��    ��: uint16_t x1,y1,x2,y2 �ַ���ʼ��������
��    ��: ��
**************************************************************************************/
void Gui_DrawFont_GBK16_Char(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t c)
{
	unsigned char i,j;
	
	if(c < 128) 		//�ж��Ƿ�Ϊһ��Ӣ���ַ�
	{
		if (c>32) c-=32; else c=0;
		Lcd_SetRegion(x,y,x+7,y+15);			//����λ�����ʾ����ʾ�����Ż�
		for(i=0;i<16;i++)						//ѭ����ʾ16���ֽ�
		{
			for(j=0;j<8;j++) 					//һ�ֽ���ģ���������ʾ
			{
				if(asc16[c*16+i]&(0x80>>j))
				{
					LCD_WriteData_16Bit(fc);
				}
				else 
				{
					if (fc!=bc) LCD_WriteData_16Bit(bc);
				}
			}
		}
	}
}


/**************************************************************************************
��������: ����Ļָ�����괦��ʾ�ָ�16����Ӣ���ַ���
��    ��: uint16_t x1,y1,x2,y2 �ַ�����ʼ��������
��    ��: ��
**************************************************************************************/
void Gui_DrawFont_GBK16(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s)
{
	unsigned char i,j;
	unsigned short k,x0;
	x0=x;

	while(*s) 
	{	
		if((*s) < 128) 		//Ӣ���ַ���ʾ
		{
			k=*s;
			if (k==13) 			//�ж��Ƿ�Ϊ�س���������
			{
				x=x0;
				y+=16;
			}
			else 
			{
				if (k>32) k-=32; else k=0;
				Lcd_SetRegion(x,y,x+7,y+15);		//����λ�����ʾ����ʾ�����Ż�
				for(i=0;i<16;i++)
				{
					for(j=0;j<8;j++)				//һ�ֽ���ģ���������ʾ
					{
						if(asc16[k*16+i]&(0x80>>j))
						{
							LCD_WriteData_16Bit(fc);
						}
						else 
						{
							if (fc!=bc) LCD_WriteData_16Bit(bc);
						}
					}
				}
				x+=8;
			}
			s++;
		}
			
		else 
		{
			for (k=0;k<hz16_num;k++) 		
			{
			  if ((hz16[k].Index[0]==*(s))&&(hz16[k].Index[1]==*(s+1)))		//�����ַ���ʾ
			  { 
					Lcd_SetRegion(x,y,x+15,y+15);				//����λ�����ʾ����ʾ�����Ż�
					for(i=0;i<32;i++)
					{
						for(j=0;j<8;j++)			//һ�ֽ���ģ���������ʾ
						{
							if(hz16[k].Msk[i]&(0x80>>j))
							{
								LCD_WriteData_16Bit(fc);
							}
							else
							{
								if (fc!=bc)
								{
									LCD_WriteData_16Bit(bc);
								}
							}
						}
					}
				}
			}
			s+=2;x+=16;
		} 
		
	}
}


/**************************************************************************************
��������: ����Ļָ�����괦��ʾ�ָ�24�ĵ���Ӣ���ַ�
��    ��: uint16_t x1,y1,x2,y2 �ַ���ʼ��������
��    ��: ��
**************************************************************************************/
void Gui_DrawFont_GBK24_Char(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t c)
{
	unsigned char i,j;
	
	if(c < 128) 		//�ж��Ƿ�Ϊһ��Ӣ���ַ�
	{
		if (c>32) c-=32; else c=0;
		Lcd_SetRegion(x,y,x+15,y+23);			//����λ�����ʾ����ʾ�����Ż�
		for(i=0;i<48;i++)						//ѭ����ʾ16���ֽ�
		{
			for(j=0;j<8;j++) 					//һ�ֽ���ģ���������ʾ
			{
				if(asc24[c*48+i]&(0x80>>j))
				{
					LCD_WriteData_16Bit(fc);
				}
				else 
				{
					if (fc!=bc) LCD_WriteData_16Bit(bc);
				}
			}
		}
	}
}



/**************************************************************************************
��������: ����Ļָ�����괦��ʾ�ָ�24����Ӣ���ַ���
��    ��: uint16_t x1,y1,x2,y2 �ַ�����ʼ��������
��    ��: ��
**************************************************************************************/
void Gui_DrawFont_GBK24(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s)
{
	unsigned char i,j;
	unsigned short k,x0;
	x0=x;

	while(*s) 
	{	
		if((*s) < 128) 		//Ӣ���ַ���ʾ
		{
			k=*s;
			if (k==13) 			//�ж��Ƿ�Ϊ�س���������
			{
				x=x0;
				y+=24;
			}
			else 
			{
				if (k>32) k-=32; else k=0;
				Lcd_SetRegion(x,y,x+15,y+23);		//����λ�����ʾ����ʾ�����Ż�
				for(i=0;i<48;i++)
				{
					for(j=0;j<8;j++)				//һ�ֽ���ģ���������ʾ
					{
						if(asc24[k*48+i]&(0x80>>j))
						{
							LCD_WriteData_16Bit(fc);
						}
						else 
						{
							if (fc!=bc) LCD_WriteData_16Bit(bc);
						}
					}
				}
				x+=12;
			}
			s++;
		}
			
		else 
		{
			for (k=0;k<hz16_num;k++) 		
			{
			  if ((hz16[k].Index[0]==*(s))&&(hz16[k].Index[1]==*(s+1)))		//�����ַ���ʾ
			  { 
					Lcd_SetRegion(x,y,x+15,y+15);				//����λ�����ʾ����ʾ�����Ż�
					for(i=0;i<32;i++)
					{
						for(j=0;j<8;j++)			//һ�ֽ���ģ���������ʾ
						{
							if(hz16[k].Msk[i]&(0x80>>j))
							{
								LCD_WriteData_16Bit(fc);
							}
							else
							{
								if (fc!=bc)
								{
									LCD_WriteData_16Bit(bc);
								}
							}
						}
					}
				}
			}
			s+=2;x+=16;
		} 		
	}
}


/**************************************************************************************
��������: ����Ļָ�����괦��ʾ�ָ�32�������ַ�
��    ��: uint16_t x1,y1,x2,y2 �ַ���ʼ��������
��    ��: ��
**************************************************************************************/
void Gui_DrawFont_Num32(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t num)
{
	uint8_t i,j,k,c;

  for(i=0;i<32;i++)
	{
		for(j=0;j<4;j++) 
		{
			c=*(sz32+num*32*4+i*4+j);
			for (k=0;k<8;k++)	
			{
		    if(c&(0x80>>k))	Gui_DrawPoint(x+j*8+k,y+i,fc);
				else if (fc!=bc) Gui_DrawPoint(x+j*8+k,y+i,bc);	
			}
		}
	}
}


/**************************************************************************************
��������: ����Ļָ�����괦��ʾ�ָ�32�������ַ���
��    ��: uint16_t x1,y1,x2,y2 �ַ���ʼ��������
��    ��: ��
**************************************************************************************/
void Gui_DrawFont_Num32Str(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s)
{
	while(*s!=0x10)
	{
		Gui_DrawFont_Num32(x, y, fc, bc, *s);
		x += 26;
		s++;
	}
}


/*---------------------------------------------------------------------------*/
/* LCD��ʾ8x16�������ͼ�꣬ռ��4��8*16�ַ�λ�ã�CΪ������0-4����Ϊ�յ�������*/
/* (���һ�����ͼ����ģ������ǰ�趨x,y���꣬����LCDд���ݣ���ɫΪ������ɫ)*/
/*---------------------------------------------------------------------------*/

void Gui_DrawBatteryIcon(uint8_t x,uint8_t y, uint16_t fc, uint16_t bc, const uint8_t *IconStr, uint8_t c)
{
 uint8_t i,j,k;

 
 k = c<<4; 	   // ȡ�ַ�����ģ�е���ʼλ��
 
 for(i=0;i<16;i++)			   // ѭ����ʾ��1�����ͼ��16�ֽ�����
 {
	for(j=0;j<8;j++)
	{
	 if (IconStr[k+i] & (0x80>>j))	// ���ݵ��ͼ����ģ����1�����ص���������ɫ���
		 Gui_DrawPoint(x+j,y+i,fc);
	 else
		 Gui_DrawPoint(x+j,y+i,bc);	// ���ݵ��ͼ����ģ����0�����ص��ñ�����ɫ���
	}// for2 end
 }// for1 end 

}


/*----------------------------------------------------------------------*/
/* LCDָ��λ����ʾ8x16���ͼ�꣬cΪ������0-4 ����Ϊ�յ絽����           */
/* (��ָ���������ָ���ַ�,fcΪ������ɫ��bcΪ������ɫ)									*/
/*----------------------------------------------------------------------*/
void Gui_DrawBatteryIconXY(uint16_t x,uint16_t y, uint16_t fc, uint16_t bc, uint8_t c)
{ 
	uint8_t i,k;
	
	k = c;
		
	if(c == 0)			//��ʾ�յ�ع����ģ
	{
		for(i=0; i<4; i++)
		{ 
			if((x+7)>LCD_XE) 	  // x����Խ����
			{
			 x = 0;
			 y += 16;
			}
			if((y+15)>LCD_YE)	  // y����Խ����
			{
			 y = 0; 
			}  
			Lcd_SetRegion (x,x+7,y,y+15);	// ����8*16��ģ��������
			Gui_DrawBatteryIcon (x, y, fc, bc, Battery0_8x16, i);		// LCD����ʾһ���յ�ع����ģ
			x += 8;							//x����������һ���ַ�λ��
		}
		return ;
	}
	else
	{
		for(i=0; i<k; i++)
		{
			if((x+7)>LCD_XE) 	  // x����Խ����
			{
			 x = 0;
			 y += 16;
			}

			if((y+15)>LCD_YE)	  // y����Խ����
			{
			 y = 0; 
			}  

			Lcd_SetRegion (x,x+7,y,y+15);	// ����8*16��ģ��������
			Gui_DrawBatteryIcon (x, y, fc, bc, Battery1_8x16, i);		// LCD����ʾһ������ع����ģ
			x += 8;							//x����������һ���ַ�λ��
		}
		
		for(i=k; i<4; i++)
		{
			if((x+7)>LCD_XE) 	  // x����Խ����
			{
			 x = 0;
			 y += 16;
			}

			if((y+15)>LCD_YE)	  // y����Խ����
			{
			 y = 0; 
			}  

			Lcd_SetRegion (x,x+7,y,y+15);	// ����8*16��ģ��������
			Gui_DrawBatteryIcon (x, y, fc, bc, Battery0_8x16, i);		// LCD����ʾһ���յ�ع����ģ
			x += 8;							//x����������һ���ַ�λ��
		}
	}
}


/*------------------------------------------------------------------------*/
/* LCDָ��λ����ʾָ����ȡ��߶ȵ�ͼƬ   												         	*/
/* x yΪͼƬ��ʼ���꣬wide high��ͼƬ���صĿ�Ⱥ͸߶�											*/
/* ImageData��ͼ������ָ��																								*/
/*------------------------------------------------------------------------*/
void Gui_DrawImage(uint16_t x,uint16_t y, uint16_t wide, uint16_t high, unsigned char *ImageData)
{
	uint32_t i;
	uint16_t data;
	
	Lcd_SetRegion(x,y,x+(wide-1),y+(high-1));
	Lcd_WriteIndex(0x2C);
	for(i=0; i<(high-1)*(wide-1); i++)
	{
		data = (uint16_t )ImageData[i*2] | (((uint16_t )ImageData[i*2+1]) << 8);
		LCD_WriteData_16Bit(data);
	}

}


