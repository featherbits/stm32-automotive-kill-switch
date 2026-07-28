/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32c0xx_hal.h"

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
#define KillSwitchSense_Pin GPIO_PIN_2
#define KillSwitchSense_GPIO_Port GPIOA
#define KillSwitchSense_EXTI_IRQn EXTI2_3_IRQn
#define PowerSignalSense_Pin GPIO_PIN_3
#define PowerSignalSense_GPIO_Port GPIOA
#define PowerSignalSense_EXTI_IRQn EXTI2_3_IRQn
#define MCUPowerSignal_Pin GPIO_PIN_4
#define MCUPowerSignal_GPIO_Port GPIOA
#define OutputPowerSignal_Pin GPIO_PIN_8
#define OutputPowerSignal_GPIO_Port GPIOA
#define ShutdownSignal_Pin GPIO_PIN_6
#define ShutdownSignal_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
