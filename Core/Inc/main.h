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
#include "stm32f4xx_hal.h"

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
#define DEBUG_CS_Pin GPIO_PIN_4
#define DEBUG_CS_GPIO_Port GPIOA
#define DEBUG_SCLK_Pin GPIO_PIN_5
#define DEBUG_SCLK_GPIO_Port GPIOA
#define DEBUG_MOSI_Pin GPIO_PIN_7
#define DEBUG_MOSI_GPIO_Port GPIOA
#define GPIO8_Pin GPIO_PIN_12
#define GPIO8_GPIO_Port GPIOE
#define GPIO7_Pin GPIO_PIN_14
#define GPIO7_GPIO_Port GPIOE
#define GPIO6_Pin GPIO_PIN_10
#define GPIO6_GPIO_Port GPIOB
#define DEBUG_BUTTON_1_Pin GPIO_PIN_12
#define DEBUG_BUTTON_1_GPIO_Port GPIOB
#define DEBUG_BUTTON_1_EXTI_IRQn EXTI15_10_IRQn
#define DEBUG_INDICATOR_1_Pin GPIO_PIN_13
#define DEBUG_INDICATOR_1_GPIO_Port GPIOB
#define DEBUG_INDICATOR_0_Pin GPIO_PIN_14
#define DEBUG_INDICATOR_0_GPIO_Port GPIOB
#define DEBUG_BUTTON_0_Pin GPIO_PIN_15
#define DEBUG_BUTTON_0_GPIO_Port GPIOB
#define DEBUG_BUTTON_0_EXTI_IRQn EXTI15_10_IRQn
#define GPIO4_Pin GPIO_PIN_8
#define GPIO4_GPIO_Port GPIOD
#define GPIO3_Pin GPIO_PIN_9
#define GPIO3_GPIO_Port GPIOD
#define GPIO2_Pin GPIO_PIN_10
#define GPIO2_GPIO_Port GPIOD
#define GPIO1_Pin GPIO_PIN_11
#define GPIO1_GPIO_Port GPIOD
#define CAN1_STBY_Pin GPIO_PIN_15
#define CAN1_STBY_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
