
#include "main.h"
#include "common.h"
#include "ssd1327.h"

void OLED_WR_DATA(u8 dat) {
	set(M_DC);
	clr(M_CS);
	HAL_SPI_Transmit(&hspi6,&dat,1,1000);
	set(M_CS);
	set(M_DC);
}
void OLED_WR_CMD(u8 dat) {
	clr(M_DC);
	clr(M_CS);
	HAL_SPI_Transmit(&hspi6,&dat,1,1000);
	set(M_CS);
	set(M_DC);
}
void OLED_WR_Byte(u8 dat,u8 cmd)
{
	u8 i;
	if(cmd)
	  set(M_DC);
	else
	  clr(M_DC);
	clr(M_CS);
	HAL_SPI_Transmit(&hspi6,&dat,1,1000);
	// for(i=0;i<8;i++)
	// {
	// 	OLED_SCL_Clr();
	// 	if(dat&0x80)
	// 	   OLED_SDA_Set();
	// 	else
	// 	   OLED_SDA_Clr();
	// 	OLED_SCL_Set();
	// 	dat<<=1;
	// }
	set(M_CS);
	set(M_DC);
}

//******************************************************************************
//    函数说明：OLED显示列的起始终止地址
//    入口数据：a  列的起始地址
//              b  列的终止地址
//    返回值：  无
//******************************************************************************
void Column_Address(u8 a, u8 b)
{
	OLED_WR_CMD(0x15);			    // Set Column Address
	OLED_WR_CMD(a);				//   Default => 0x00
	OLED_WR_CMD(b);				//   Default => 0x77
}

//******************************************************************************
//    函数说明：OLED显示行的起始终止地址
//    入口数据：a  行的起始地址
//              b  行的终止地址
//    返回值：  无
//******************************************************************************
void Row_Address(u8 a, u8 b)
{
	OLED_WR_CMD(0x75);			// Set Row Address
	OLED_WR_CMD(a);				//   Default => 0x00
	OLED_WR_CMD(b);			//   Default => 0x3F
}

//******************************************************************************
//    函数说明：OLED清屏显示
//    入口数据：无
//    返回值：  无
//******************************************************************************
void OLED_Clear(u8 x1,u8 y1,u8 x2,u8 y2,u8 color)
{
	u8 i,m=0;
	x1/=2;x2/=2;
	Column_Address(x1,x2-1);
	Row_Address(y1,y2-1);
	for (m=0;m<y2-y1;m++)
	{
		for (i=0;i<x2-x1;i++)
		{
			 OLED_WR_DATA(color);
		}
	}
}


//初始化
void Init_ssd1327(void)
{
	OLED_RES_Clr();
	HAL_Delay(200);
	OLED_RES_Set();

	OLED_WR_Byte(0xae,OLED_CMD);//Set display off
	OLED_WR_Byte(0xa0,OLED_CMD);//Set re-map
	if(USE_HORIZONTAL){OLED_WR_Byte(0x66,OLED_CMD);}
	else{OLED_WR_Byte(0x55,OLED_CMD);}
	OLED_WR_Byte(0xa1,OLED_CMD);//Set display start line
	OLED_WR_Byte(0x00,OLED_CMD);
	OLED_WR_Byte(0xa2,OLED_CMD);//Set display offset
	OLED_WR_Byte(0x00,OLED_CMD);
	OLED_WR_Byte(0xa4,OLED_CMD);//Normal Display
	OLED_WR_Byte(0xa8,OLED_CMD);//Set multiplex ratio
	OLED_WR_Byte(0x7f,OLED_CMD);
	OLED_WR_Byte(0xab,OLED_CMD);//Function Selection A
	OLED_WR_Byte(0x01,OLED_CMD);//Enable internal VDD regulator
	OLED_WR_Byte(0x81,OLED_CMD);//Set contrast
	OLED_WR_Byte(0x77,OLED_CMD);
	OLED_WR_Byte(0xb1,OLED_CMD);//Set Phase Length
	OLED_WR_Byte(0x31,OLED_CMD);
	OLED_WR_Byte(0xb3,OLED_CMD);//Set Front Clock Divider /Oscillator Frequency
	OLED_WR_Byte(0xb1,OLED_CMD);
  OLED_WR_Byte(0xb5,OLED_CMD);//
  OLED_WR_Byte(0x03,OLED_CMD);//0X03 enable
	OLED_WR_Byte(0xb6,OLED_CMD);//Set Second pre-charge Period
	OLED_WR_Byte(0x0d,OLED_CMD);
	OLED_WR_Byte(0xbc,OLED_CMD);//Set Pre-charge voltage
	OLED_WR_Byte(0x07,OLED_CMD);
	OLED_WR_Byte(0xbe,OLED_CMD);//Set VCOMH
	OLED_WR_Byte(0x07,OLED_CMD);
	OLED_WR_Byte(0xd5,OLED_CMD);//Function Selection B
	OLED_WR_Byte(0x02,OLED_CMD);//Enable second pre-charge
	OLED_Clear(0,0,127,127,0x00);
	OLED_WR_Byte(0xaf,OLED_CMD);//Display on
}

