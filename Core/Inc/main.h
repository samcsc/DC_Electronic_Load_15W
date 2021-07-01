/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32g4xx_hal.h"

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
#define BT_A_Pin GPIO_PIN_4
#define BT_A_GPIO_Port GPIOA
#define BT_A_EXTI_IRQn EXTI4_IRQn
#define BT_INC_Pin GPIO_PIN_5
#define BT_INC_GPIO_Port GPIOA
#define BT_INC_EXTI_IRQn EXTI9_5_IRQn
#define BT_DEC_Pin GPIO_PIN_6
#define BT_DEC_GPIO_Port GPIOA
#define BT_DEC_EXTI_IRQn EXTI9_5_IRQn
#define LED_PC_Pin GPIO_PIN_7
#define LED_PC_GPIO_Port GPIOA
#define LED_RUN_Pin GPIO_PIN_4
#define LED_RUN_GPIO_Port GPIOC
#define LED_100_Pin GPIO_PIN_5
#define LED_100_GPIO_Port GPIOC
#define LED_500_Pin GPIO_PIN_0
#define LED_500_GPIO_Port GPIOB
#define LED_1000_Pin GPIO_PIN_1
#define LED_1000_GPIO_Port GPIOB
#define LED_2000_Pin GPIO_PIN_2
#define LED_2000_GPIO_Port GPIOB
#define CS_DAC_Pin GPIO_PIN_6
#define CS_DAC_GPIO_Port GPIOC
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
