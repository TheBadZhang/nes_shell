#ifndef __TBZ_COMMON_H__
#define __TBZ_COMMON_H__
#include "main.h"
#include "cmsis_os.h"

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern SPI_HandleTypeDef hspi4;
extern SPI_HandleTypeDef hspi6;
extern DMA_HandleTypeDef hdma_spi4_tx;
extern DMA_HandleTypeDef hdma_spi6_tx;
extern PCD_HandleTypeDef hpcd_USB_OTG_HS;

extern TIM_HandleTypeDef htim15;
extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim17;
extern RTC_HandleTypeDef hrtc;
extern RNG_HandleTypeDef hrng;

#endif