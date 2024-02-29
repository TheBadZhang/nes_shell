/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_3
#define LED_GPIO_Port GPIOE
#define K1_Pin GPIO_PIN_13
#define K1_GPIO_Port GPIOC
#define M_SCK_Pin GPIO_PIN_5
#define M_SCK_GPIO_Port GPIOA
#define M_SDA_Pin GPIO_PIN_7
#define M_SDA_GPIO_Port GPIOA
#define GAIN_Pin GPIO_PIN_4
#define GAIN_GPIO_Port GPIOC
#define AR_Pin GPIO_PIN_1
#define AR_GPIO_Port GPIOB
#define LCD_LED_Pin GPIO_PIN_10
#define LCD_LED_GPIO_Port GPIOE
#define CS1_Pin GPIO_PIN_11
#define CS1_GPIO_Port GPIOE
#define SCK1_Pin GPIO_PIN_12
#define SCK1_GPIO_Port GPIOE
#define DC1_Pin GPIO_PIN_13
#define DC1_GPIO_Port GPIOE
#define SDA1_Pin GPIO_PIN_14
#define SDA1_GPIO_Port GPIOE
#define RST1_Pin GPIO_PIN_15
#define RST1_GPIO_Port GPIOE
#define C5_Pin GPIO_PIN_12
#define C5_GPIO_Port GPIOB
#define C4_Pin GPIO_PIN_13
#define C4_GPIO_Port GPIOB
#define C3_Pin GPIO_PIN_14
#define C3_GPIO_Port GPIOB
#define C2_Pin GPIO_PIN_15
#define C2_GPIO_Port GPIOB
#define R3_Pin GPIO_PIN_8
#define R3_GPIO_Port GPIOD
#define R2_Pin GPIO_PIN_9
#define R2_GPIO_Port GPIOD
#define R1_Pin GPIO_PIN_10
#define R1_GPIO_Port GPIOD
#define C1_Pin GPIO_PIN_15
#define C1_GPIO_Port GPIOA
#define M_RST_Pin GPIO_PIN_0
#define M_RST_GPIO_Port GPIOD
#define M_CS_Pin GPIO_PIN_5
#define M_CS_GPIO_Port GPIOD
#define SPI1_CS_Pin GPIO_PIN_6
#define SPI1_CS_GPIO_Port GPIOD
#define M_DC_Pin GPIO_PIN_5
#define M_DC_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
