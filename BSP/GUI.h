/****************************************
ST7735 SPI 1.8" TFT driver header file
Ver 1.0

128*160
SPI suggest 16M speed
LCD TFT function
****************************************/
#ifndef __GUI_h
#define __GUI_h 

#include "Lcd_Driver.h"

//Ƥ����ɫ�ṹ��
//struct SkinType{
//	char     *skin_name;
//	uint16_t main_fc;					//������ǰ��ɫ
//	uint16_t main_bc;					//�����汳��ɫ
//	uint16_t statusup_fc;			//�ϱ�����ǰ��ɫ
//	uint16_t statusup_bc;			//�ϱ���������ɫ
//	uint16_t statusdown_fc;		//�±�����ǰ��ɫ
//	uint16_t statusdown_bc;		//�±���������ɫ
//	uint16_t lab_fc;					//��ǰ���߱�ǩǰ��ɫ
//	uint16_t lab_dfc;					//��ǰ���߱�ǩʧ��ǰ��ɫ
//	uint16_t lab_bc;					//��ǰ���߱�ǩ����ɫ
//	uint16_t menubar_fc;			//�˵�����ǰ��ɫ
//	uint16_t menubar_bc;			//�˵����ⱳ��ɫ
//	uint16_t menu_fc;					//�˵�����ǰ��ɫ
//	uint16_t menu_bc;					//�˵����汳��ɫ
//};
//
//extern struct SkinType Skin_Current;
//extern struct SkinType Skin[4];


uint16_t LCD_BGR2RGB(uint16_t c);
void Gui_Circle(uint16_t X,uint16_t Y,uint16_t R,uint16_t fc); 
void Gui_FillCircle(uint16_t x0,uint16_t y0,uint16_t r,uint16_t fc);
void Gui_RoundRect(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t r, uint16_t fc);
void Gui_DrawFont_GBK8_Char(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t c);
void Gui_DrawFont_GBK8(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *str);
void Gui_DrawFont_GBK16_Char(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t s);
void Gui_DrawFont_GBK16(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s);
void Gui_DrawFont_GBK24_Char(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t c);
void Gui_DrawFont_GBK24(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s);
void Gui_DrawFont_Num32(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t num);
void Gui_DrawFont_Num32Str(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s);
void Gui_DrawBatteryIconXY(uint16_t x,uint16_t y, uint16_t fc, uint16_t bc, uint8_t c);
void Gui_DrawImage(uint16_t x,uint16_t y, uint16_t wide, uint16_t high, unsigned char *ImageData);


#endif
