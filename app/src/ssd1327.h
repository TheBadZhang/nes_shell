#ifndef __OLED_H
#define __OLED_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
typedef uint8_t u8;
typedef uint32_t u32;

#include "trick.h"
#include "common.h"
#define USE_HORIZONTAL 0  //������ʾ���� 0��������ʾ��1����ת180����ʾ


#define OLED_RES_Clr() clr(M_RST)
#define OLED_RES_Set() set(M_RST)

#define OLED_DC_Clr()  clr(M_DC)
#define OLED_DC_Set()  set(M_DC)

#define OLED_CS_Clr()  clr(M_CS)
#define OLED_CS_Set()  set(M_CS)


#define OLED_CMD  0	//д����
#define OLED_DATA 1	//д����

void OLED_WR_Byte(u8 dat,u8 cmd);
void Column_Address(u8 a, u8 b);
void Row_Address(u8 a, u8 b);
void OLED_Clear(u8 x1,u8 y1,u8 x2,u8 y2,u8 color);
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 sizey);
void OLED_ShowString(u8 x,u8 y,u8 *dp,u8 sizey);
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 sizey);
void OLED_ShowChinese(u8 x,u8 y,u8 num,u8 sizey);
void OLED_DrawBMP(u8 x,u8 y,u8 width,u8 height,const u8 *BMP);
void Init_ssd1327(void);

#ifdef __cplusplus
}
#endif

#endif

