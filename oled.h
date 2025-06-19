#ifndef __OLED_H
#define __OLED_H			  	 
#include "sys.h"
#include "stdlib.h"	    

 
/*********************************************
//oled
//7��0.96��oled ���� :
GND	->GND			2   
VCC	->3.3V			1
D0	->			    3		PB10
D1	->			    4		PA1
RES	->				5		PA0
DC	->				6       PD1
CS	->				7		PD0
************************************************/							   
 

//OLEDģʽ����
//0: 4�ߴ���ģʽ  ��ģ���BS1��BS2����GND��
//1: ����8080ģʽ ��ģ���BS1��BS2����VCC��
#define OLED_MODE 	0 
		    						  
//-----------------OLED�˿ڶ���----------------  					
#define OLED_WR 	PBout(12)  //D0	  
#define OLED_RD 	PBout(13)  //D1
#define OLED_RST  PBout(14)	//RST
#define OLED_RS 	PBout(15)  //DC   
#define OLED_CS 	PAout(8)   //CS
 
//ʹ��4�ߴ��нӿ�ʱʹ�� 
#define OLED_SCLK 	PBout(12)
#define OLED_SDIN 	PBout(13)
		     
#define OLED_CMD  	0		//д����
#define OLED_DATA 	1		//д����
//OLED�����ú���
void OLED_WR_Byte(u8 dat,u8 cmd);	    
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Refresh_Gram(void);		   
							   		    
void OLED_Init(void);
void OLED_Clear(void);
void OLED_Clear_NOupdate(void);
void OLED_DrawPoint(u8 x,u8 y,u8 t);
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot);
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size,u8 mode);
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size);
void OLED_ShowString(u8 x,u8 y,const u8 *p,u8 size);	
void OLED_Show_Font(u16 x,u16 y,u8 fnum);
void OLED_Show_Image(u8 fnum);

void Show_Str(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode);

u16 LCD_BGR2RGB(u16 c);
void Gui_Circle(u16 X,u16 Y,u16 R,u16 fc); 
void Gui_DrawLine(u16 x0, u16 y0,u16 x1, u16 y1,u16 Color);  
void Gui_box(u16 x, u16 y, u16 w, u16 h,u16 bc);
void Gui_box2(u16 x,u16 y,u16 w,u16 h, u8 mode);
void DisplayButtonDown(u16 x1,u16 y1,u16 x2,u16 y2);
void DisplayButtonUp(u16 x1,u16 y1,u16 x2,u16 y2);
void Gui_DrawFont_GBK12(u16 x, u16 y, u16 fc, u16 bc, u8 *s);
void Gui_DrawFont_GBK16(u16 x, u16 y, u16 fc, u16 bc, u8 *s);
void Gui_DrawFont_GBK24(u16 x, u16 y, u16 fc, u16 bc, u8 *s);
void Gui_DrawFont_Num32(u16 x, u16 y, u16 fc, u16 bc, u16 num) ;



#endif  
	 



