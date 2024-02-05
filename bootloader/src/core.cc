#include "core.h"
#include "common.h"


#include <span>
#include <functional>
#include <cstdarg>
#include <cstdio>

#include "hardware_timer.h"
#include "sys.h"
#include "trick.h"
#include "st7735.h"

#include "w25qxx_ospi.h"
#include "w25q_ospi.h"


uint8_t OSPI_NOR_ReadData2[1024] {0};

/* 开关全局中断的宏 */
#define ENABLE_INT()	__set_PRIMASK(0)	/* 使能全局中断 */
#define DISABLE_INT()	__set_PRIMASK(1)	/* 禁止全局中断 */

void BspQspiBoot_JumpToApp(__IO uint32_t AppAddr = 0x90000000/* APP 地址 */) {
	uint32_t i = 0;
	void (*AppJump)(void);         /* 声明一个函数指针 */

	/* 关闭全局中断 */
	DISABLE_INT();

	/* 设置所有时钟到默认状态，使用HSI时钟 */
	HAL_RCC_DeInit();

	/* 关闭滴答定时器，复位到默认值 */
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;

	/* 关闭所有中断，清除所有中断挂起标志 */
	for (i = 0; i < 8; i++) {
		NVIC->ICER[i]=0xFFFFFFFF;
		NVIC->ICPR[i]=0xFFFFFFFF;
	}

	/* 使能全局中断 */
	ENABLE_INT();

	/* 跳转到应用程序，首地址是MSP，地址+4是复位中断服务程序地址 */
	AppJump = (void (*)(void)) (*((uint32_t *) (AppAddr + 4)));

	/* 设置主堆栈指针 */
	__set_MSP(*(uint32_t *)AppAddr);

	/* 在RTOS工程，这条语句很重要，设置为特权级模式，使用MSP指针 */
	__set_CONTROL(0);

	/* 跳转到系统BootLoader */
	AppJump();

	/* 跳转成功的话，不会执行到这里，用户可以在这里添加代码 */
	while (1)
	{

	}
}

using pFunction = void (*)(void);
uint8_t __cpu_goto_app(uint32_t Addr)
{
	pFunction Jump_To_Application;
	__IO uint32_t JumpAddress;

	if (((*(__IO uint32_t*)Addr) & 0x2FFE0000 ) == 0x20000000)
	{
		/* 地址要偏移4 */
		JumpAddress = *(__IO uint32_t*) (Addr + 4);
		Jump_To_Application = (pFunction) JumpAddress;

		/* 使用app的栈 */
		__set_MSP(*(__IO uint32_t*) Addr);

		/* 跳转到用户函数入口地址 */
		Jump_To_Application();
	}
	return 1;
}

char buf[128];



#define VECT_TAB_OFFSET      (0x00000000UL)
#define APPLICATION_ADDRESS  (0x90000000UL)

typedef  void (*pFunction)(void);
pFunction JumpToApplication;

void jump2(void) {

	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;
	for(uint8_t i = 0; i < 8; i++)
	{
		NVIC->ICER[i]=0xFFFFFFFF;
		NVIC->ICPR[i]=0xFFFFFFFF;
	}
	__set_CONTROL(0);
	__disable_irq();
	__set_PRIMASK(1);
	HAL_GPIO_DeInit(GPIOE, GPIO_PIN_3);
	JumpToApplication = (pFunction) (*(__IO uint32_t*)(APPLICATION_ADDRESS + 4));
	__set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
	JumpToApplication();
}


static void JumpToApp(void) {
	uint32_t i=0;
	void (*SysMemBootJump)(void);        /* 声明一个函数指针 */
	__IO uint32_t BootAddr = 0x90000000; /* STM32H7的系统BootLoader地址 */


	/* 关闭全局中断 */
	DISABLE_INT();

	/* 关闭滴答定时器，复位到默认值 */
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;

	/* 设置所有时钟到默认状态，使用HSI时钟 */
	HAL_RCC_DeInit();

	/* 关闭所有中断，清除所有中断挂起标志 */
	for (i = 0; i < 8; i++)
	{
			NVIC->ICER[i]=0xFFFFFFFF;
			NVIC->ICPR[i]=0xFFFFFFFF;
	}

	/* 使能全局中断 */
	ENABLE_INT();

	/* 跳转到系统BootLoader，首地址是MSP，地址+4是复位中断服务程序地址 */
	SysMemBootJump = (void (*)(void)) (*((uint32_t *) (BootAddr + 4)));

	/* 设置主堆栈指针 */
	__set_MSP(*(uint32_t *)BootAddr);

	/* 在RTOS工程，这条语句很重要，设置为特权级模式，使用MSP指针 */
	__set_CONTROL(0);

	/* 跳转到系统BootLoader */
	SysMemBootJump();

	/* 跳转成功的话，不会执行到这里，用户可以在这里添加代码 */
	while (1)
	{

	}
}

#include "oled.h"



uint8_t screen_buffer[128*128/2];
unsigned char ucTemp[128];

static void oledWrite(unsigned char *pData, int iLen)
{
	clr(M_CS);
	HAL_SPI_Transmit(&hspi6, pData, iLen, 1000);
	set(M_CS);
} /* oledWrite() */
// static void ssd1327WriteCommandBlock()
static void ssd1327WriteDataBlock(unsigned char *ucBuf, int iLen)
{
	clr(M_DC);
  oledWrite(ucTemp, iLen);
} /* ssd1327WriteDataBlock() */
//
// Send commands to position the "cursor" (aka memory write address)
// to the given row and column as well as the ending col/row
//
static void ssd1327SetPosition(int x, int y, int cx, int cy)
{
	unsigned char bbbuf[8];

	bbbuf[0] = 0x00; // command introducer
	bbbuf[1] = 0x15; // column start/end
	bbbuf[2] = x/2; // start address
	bbbuf[3] = (uint8_t)(((x+cx)/2)-1); // end address
	bbbuf[4] = 0x75; // row start/end
	//    if (oled_type == OLED_96x96)
	//       y += 32;
	bbbuf[5] = y; // start row
	bbbuf[6] = y+cy-1; // end row
	set(M_DC);
	oledWrite(bbbuf+1, 6);
	clr(M_DC);
} /* ssd1327SetPosition() */

#include "st7789.hpp"
tbz::device::screen::st7789 st7789;


void core(void) {

	tbz::tft::st7735::init();
	tbz::tft::st7735::fillScreen(tbz::tft::st7735::COLOR::BLACK);
	tbz::tft::st7735::writeString(0, 0, "Hello World!", Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	tbz::tft::st7735::writeString(0, 10, "OSPI FLASH INIT...", Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	auto retval = OSPI_NOR_Init();
	tbz::timer::delay_ms(1);
	if (retval == 0) {
		tbz::tft::st7735::writeString(0, 20, "FLASH INIT OK", Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	} else {
		tbz::tft::st7735::writeString(0, 20, "FLASH INIT FAIL", Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
		sprintf(buf, "ret:%lX", retval);
		tbz::tft::st7735::writeString(0, 30, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
		return ;
	}


	retval = OSPI_NOR_ReadSTR(OSPI_NOR_ReadData2, 0, 1024);
	sprintf(buf, "data:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		OSPI_NOR_ReadData2[0], OSPI_NOR_ReadData2[1], OSPI_NOR_ReadData2[2],
		OSPI_NOR_ReadData2[3], OSPI_NOR_ReadData2[4], OSPI_NOR_ReadData2[5],
		OSPI_NOR_ReadData2[6], OSPI_NOR_ReadData2[7], OSPI_NOR_ReadData2[8],
		OSPI_NOR_ReadData2[9], OSPI_NOR_ReadData2[10], OSPI_NOR_ReadData2[11],
		OSPI_NOR_ReadData2[12], OSPI_NOR_ReadData2[13], OSPI_NOR_ReadData2[14],
		OSPI_NOR_ReadData2[15], OSPI_NOR_ReadData2[16], OSPI_NOR_ReadData2[17],
		OSPI_NOR_ReadData2[18], OSPI_NOR_ReadData2[19], OSPI_NOR_ReadData2[20],
		OSPI_NOR_ReadData2[21], OSPI_NOR_ReadData2[22], OSPI_NOR_ReadData2[23]);
	tbz::tft::st7735::writeString(0, 59, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);

	tbz::tft::st7735::writeString(0, 30, "MEMORY MAPPED MODE...", Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	retval = OSPI_NOR_EnableSTRMemoryMappedMode();
	if (retval == 0) {
		tbz::tft::st7735::writeString(0, 40, "MEMORY MAPPED OK", Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	} else {
		tbz::tft::st7735::writeString(0, 40, "MEMORY MAPPED FAIL", Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
		sprintf(buf, "ret:%lX", retval);
		tbz::tft::st7735::writeString(0, 50, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
		return ;
	}
	tbz::tft::st7735::writeString(0, 49, "JUMP TO APP...", Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	BspQspiBoot_JumpToApp();
	// JumpToApp();
	// jump2();

	// U8G2_SSD1327_MIDAS_128X128_f_4W_HW_SPI u8g2(U8G2_R0);
	// u8g2.begin();

	st7789
		.setup()
		.setAddressWindow(0, 0, 240, 320)
		.fillScreen(0xffff)
		.setDisplayOn();
	while (true) {

		flip(LED);
		tbz::timer::delay_ms(50);
		// ssd1327SetPosition(0, 0, 64, 64);
		// auto ptr = u8g2.getBufferPtr();
		// for (int i = 0; i < 128*128/8; i++) {
		// 	// auto pixe = ptr[i];
		// 	auto pixe = 0x55;
		// 	screen_buffer[1+i*4] = (pixe & 0x01) | ((pixe & 0x02) << 3);
		// 	screen_buffer[1+i*4+1] = ((pixe & 0x04) >> 2) | ((pixe & 0x08) << 1);
		// 	screen_buffer[1+i*4+2] = ((pixe & 0x10) >> 4) | ((pixe & 0x20) >> 1);
		// 	screen_buffer[1+i*4+3] = ((pixe & 0x40) >> 6) | ((pixe & 0x80) >> 3);
		// }
		// std::fill(screen_buffer, screen_buffer+64*64/2, 0x50);
		// // ssd1327WriteDataBlock(screen_buffer, 64*64/2);
		// for (int i = 0; i < 32; i ++) {
		// 	ssd1327WriteDataBlock(screen_buffer+i*64/2, 64/2);
		// }
	}

}



/*


	auto retval = OSPI_NOR_Init();
	// int retval = 42;
	tbz::timer::delay_ms(1);
	// sprintf(buf, "init_ret:%lX-id:%lX", retval, OSPI_NOR_ID);
	// tbz::tft::st7735::writeString(0, 10, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	// // OSPI_NOR_WriteEnable();
	// // OSPI_NOR_ChipErase();
	// OSPI_NOR_BlockErase(0);
	// // OSPI_NOR_BlockErase(1);
	// // OSPI_NOR_BlockErase(2);
	// // OSPI_NOR_BlockErase(3);
	// // OSPI_NOR_ChipErase();
	// // tbz::timer::delay_ms(10);
	// // OSPI_NOR_Mode = W25Qxx_QPI_MODE;
	// retval = OSPI_NOR_WriteNoCheck(OSPI_NOR_WriteData2, 0, 64);
	// sprintf(buf, "write_ret:%lX", retval);
	// tbz::tft::st7735::writeString(0, 20, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	// tbz::timer::delay_ms(1);
	// // // OSPI_NOR_Mode = W25Qxx_SPI_MODE;
	// // for(int i = 0; i < 40; i++) {
	// // 	retval = OSPI_NOR_ReadSTR(OSPI_NOR_ReadData2, i*256, 256);
	// // 	HAL_UART_Transmit(&huart1, OSPI_NOR_ReadData2, 256, 1000);
	// // 	tbz::timer::delay_ms(5);
	// // }
	// // OSPI_NOR_Mode = W25Qxx_QPI_MODE;
	retval = OSPI_NOR_ReadSTR(OSPI_NOR_ReadData2, 0, 1024);
	// OSPI_NOR_Mode = W25Qxx_SPI_MODE;
	// HAL_UART_Transmit(&huart1, OSPI_NOR_ReadData2, 1024, 1000);
	// sprintf(buf, "read_ret:%lX", retval);
	// tbz::tft::st7735::writeString(0, 30, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	// tbz::timer::delay_ms(1);
	sprintf(buf, "data:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		OSPI_NOR_ReadData2[0], OSPI_NOR_ReadData2[1], OSPI_NOR_ReadData2[2],
		OSPI_NOR_ReadData2[3], OSPI_NOR_ReadData2[4], OSPI_NOR_ReadData2[5],
		OSPI_NOR_ReadData2[6], OSPI_NOR_ReadData2[7], OSPI_NOR_ReadData2[8],
		OSPI_NOR_ReadData2[9], OSPI_NOR_ReadData2[10], OSPI_NOR_ReadData2[11]);
	tbz::tft::st7735::writeString(0, 10, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	// uint8_t reg[3];
	// retval = OSPI_NOR_ReadAllRegister(reg);
	// sprintf(buf, "reg:%02x-%02x-%02x", reg[0], reg[1], reg[2]);
	// tbz::tft::st7735::writeString(0, 50, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	retval = OSPI_NOR_EnableSTRMemoryMappedMode();
	sprintf(buf, "enable_ret:%lX", retval);
	tbz::tft::st7735::writeString(0, 50, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	tbz::timer::delay_ms(5);
	BspQspiBoot_JumpToApp();
*/
/*


	auto retval = OSPI_NOR_Init();
	// int retval = 42;
	tbz::timer::delay_ms(1);
	sprintf(buf, "init_ret:%lX-id:%lX", retval, OSPI_NOR_ID);
	tbz::tft::st7735::writeString(0, 10, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	// OSPI_NOR_WriteEnable();
	// OSPI_NOR_ChipErase();
	OSPI_NOR_BlockErase(0);
	// OSPI_NOR_BlockErase(1);
	// OSPI_NOR_BlockErase(2);
	// OSPI_NOR_BlockErase(3);
	// OSPI_NOR_ChipErase();
	// tbz::timer::delay_ms(10);
	// OSPI_NOR_Mode = W25Qxx_QPI_MODE;
	retval = OSPI_NOR_WriteNoCheck(OSPI_NOR_WriteData2, 0, 64);
	sprintf(buf, "write_ret:%lX", retval);
	tbz::tft::st7735::writeString(0, 20, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	tbz::timer::delay_ms(1);
	// // OSPI_NOR_Mode = W25Qxx_SPI_MODE;
	// for(int i = 0; i < 40; i++) {
	// 	retval = OSPI_NOR_ReadSTR(OSPI_NOR_ReadData2, i*256, 256);
	// 	HAL_UART_Transmit(&huart1, OSPI_NOR_ReadData2, 256, 1000);
	// 	tbz::timer::delay_ms(5);
	// }
	// OSPI_NOR_Mode = W25Qxx_QPI_MODE;
	retval = OSPI_NOR_ReadSTR(OSPI_NOR_ReadData2, 0, 1024);
	// OSPI_NOR_Mode = W25Qxx_SPI_MODE;
	HAL_UART_Transmit(&huart1, OSPI_NOR_ReadData2, 1024, 1000);
	sprintf(buf, "read_ret:%lX", retval);
	tbz::tft::st7735::writeString(0, 30, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	tbz::timer::delay_ms(1);
	sprintf(buf, "data:%x-%x-%x", OSPI_NOR_ReadData2[0], OSPI_NOR_ReadData2[1], OSPI_NOR_ReadData2[2]);
	tbz::tft::st7735::writeString(0, 40, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	uint8_t reg[3];
	retval = OSPI_NOR_ReadAllRegister(reg);
	sprintf(buf, "reg:%02x-%02x-%02x", reg[0], reg[1], reg[2]);
	tbz::tft::st7735::writeString(0, 50, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);







ospi id 读正常，其他不正常（后来发现是芯片被锁了，其实读写都正常，但是没能用这个库解锁flash）

	extern uint32_t OSPI_NOR_ID;
	extern W25Qxx_Interface_t OSPI_NOR_Mode;
	extern uint32_t errorcount;
	OSPI_NOR_Test();
	sprintf(buf, "errorcount:%d", errorcount);
	tbz::tft::st7735::writeString(0, 60, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);

	auto retval = OSPI_NOR_Init();
	// int retval = 42;
	tbz::timer::delay_ms(1);
	sprintf(buf, "init_ret:%lX-id:%lX", retval, OSPI_NOR_ID);
	tbz::tft::st7735::writeString(0, 10, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	// OSPI_NOR_ChipErase();
	// OSPI_NOR_WriteEnable();
	// OSPI_NOR_SectorErase(0);
	OSPI_NOR_BlockErase(0);
	// OSPI_NOR_ChipErase();
	tbz::timer::delay_ms(10);
	// OSPI_NOR_Mode = W25Qxx_QPI_MODE;
	retval = OSPI_NOR_WriteNoCheck(OSPI_NOR_WriteData2, 0, 64);
	sprintf(buf, "write_ret:%lX", retval);
	tbz::tft::st7735::writeString(0, 20, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	tbz::timer::delay_ms(1);
	// OSPI_NOR_Mode = W25Qxx_SPI_MODE;
	retval = OSPI_NOR_ReadSTR(OSPI_NOR_ReadData2, 0, 1024);
	sprintf(buf, "read_ret:%lX", retval);
	tbz::tft::st7735::writeString(0, 30, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	tbz::timer::delay_ms(1);
	sprintf(buf, "data:%x-%x-%x", OSPI_NOR_ReadData2[0], OSPI_NOR_ReadData2[1], OSPI_NOR_ReadData2[2]);
	tbz::tft::st7735::writeString(0, 40, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	uint8_t reg[3];
	retval = OSPI_NOR_ReadAllRegister(reg);
	sprintf(buf, "reg:%02x-%02x-%02x", reg[0], reg[1], reg[2]);
	tbz::tft::st7735::writeString(0, 50, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	HAL_UART_Transmit(&huart1, OSPI_NOR_ReadData2, 1024, 1000);

*/


/*
spi1 flash 读写，成功，正常
	W25Qx_Init();
	uint16_t id2;
	tbz::timer::delay_ms(1000);
	W25Qx_Erase_Block(0);
	tbz::timer::delay_ms(500);
	W25Qx_Read_ID(&id2);
	sprintf(buf, "id:%lX", id2);
	tbz::tft::st7735::writeString(0, 20, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	tbz::timer::delay_ms(1000);
	W25Qx_Read_JEDECID(&id);
	sprintf(buf, "jedecid:%lX", id);
	tbz::tft::st7735::writeString(0, 30, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	retval = W25Qx_Write((uint8_t*)"Hello World!", 0, 13);
	sprintf(buf, "ret:%lX", retval);
	tbz::tft::st7735::writeString(0, 40, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	tbz::timer::delay_ms(500);
	retval = W25Qx_Read((uint8_t*)OSPI_NOR_ReadData2, 0, 128);
	tbz::timer::delay_ms(500);
	sprintf(buf, "ret:%lX", retval);
	tbz::tft::st7735::writeString(0, 50, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);
	HAL_UART_Transmit(&huart1, OSPI_NOR_ReadData2, 128, 1000);
	// sprintf(buf, "ret:%lX-data:%x-%x-%x", retval, OSPI_NOR_ReadData2[0], OSPI_NOR_ReadData2[1], OSPI_NOR_ReadData2[2]);
	// tbz::tft::st7735::writeString(0, 20, buf, Font_7x10, tbz::tft::st7735::COLOR::WHITE, tbz::tft::st7735::COLOR::BLACK);


*/