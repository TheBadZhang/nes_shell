#include "core.h"

#include "common.h"
// #include "oled.h"
// #include "sys.h"
#include "trick.h"
#include <cstring>
#include <cstdint>
#include <cstdio>
#include "InfoNES.h"
#include "st7789.hpp"
tbz::device::screen::st7789 st7789;

// uint8_t screen_buffer_16bit[240*320*2] __attribute__((section(".ramd1section")));
int screen_width = 128;
int screen_height = 128;
void core(void) {

	set(LCD_BLK);

	st7789
		.setup(::tbz::device::screen::R2)
		.setBuffer((uint8_t*)WorkFrame)
		.setAddressWindow(0, 0, 320, 240)
		.fillScreen(0x0000)
		.fillScreen([]() -> u16 {
			return rand();
		})
		.fillScreenf([](float x, float y) -> u16 {
			return abs(sin(x*10)*cos(y*10))*0xffff;
		})
		.setAddressWindow2((320-256)/2, (240-240)/2, 256, 240);

	HAL_TIM_Base_Start_IT(&htim16);
	if (InfoNES_Load(NULL) == 0) {
		// FrameSkip = 3;
	// if (InfoNES_Load("./Tanks .nes") == 0) {
		InfoNES_Main();
	}
	while (true) {


		flip(LED);
		HAL_Delay(500);
	}

}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	static int flag = 1;
	if (htim == (&htim16)) {
	// start_trans_data();
	// if ((HAL_SPI_GetState(&hspi5) == HAL_SPI_STATE_READY))
	// HAL_SPI_Transmit_DMA(&hspi5, (uint8_t*)WorkFrame+st7789.buffer_single_trans_size, st7789.buffer_single_trans_size);
		if (HAL_SPI_GetState(&hspi5) == HAL_SPI_STATE_READY)
		if (flag) {

			flip(LED);
			st7789
				.setBuffer((uint8_t*)(WorkFrame))
				.setAddressWindow2((320-256)/2, (240-240)/2, 256, 240)
				.sendHalfBuffer1_nodelay();
			flag = 0;
		} else {
			st7789.sendHalfBuffer2_nodelay();
			flag = 1;
		}
	}
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
	if (hspi == &hspi5) {
		// HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
	}
}