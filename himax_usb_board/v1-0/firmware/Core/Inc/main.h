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
#include "stm32f7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdint.h"
#include "stdbool.h"
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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
/**
 * Putch onto UART7 via an interrupt-deferred queue.
 */
void putch(char);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define led0_Pin GPIO_PIN_13
#define led0_GPIO_Port GPIOC
#define led1_Pin GPIO_PIN_14
#define led1_GPIO_Port GPIOC
#define led2_Pin GPIO_PIN_15
#define led2_GPIO_Port GPIOC
#define gpio0_Pin GPIO_PIN_6
#define gpio0_GPIO_Port GPIOF
#define gpio1_Pin GPIO_PIN_7
#define gpio1_GPIO_Port GPIOF
#define gpio2_Pin GPIO_PIN_8
#define gpio2_GPIO_Port GPIOF
#define gpio3_Pin GPIO_PIN_9
#define gpio3_GPIO_Port GPIOF
#define gpio4_Pin GPIO_PIN_10
#define gpio4_GPIO_Port GPIOF
#define hm01b0_int_Pin GPIO_PIN_0
#define hm01b0_int_GPIO_Port GPIOA
#define hm01b0_trig_Pin GPIO_PIN_1
#define hm01b0_trig_GPIO_Port GPIOA
#define hm01b0_mclk_Pin GPIO_PIN_2
#define hm01b0_mclk_GPIO_Port GPIOA
#define hm0360_mclk_Pin GPIO_PIN_11
#define hm0360_mclk_GPIO_Port GPIOE
#define hm0360_trig_Pin GPIO_PIN_13
#define hm0360_trig_GPIO_Port GPIOE
#define hm0360_int_Pin GPIO_PIN_14
#define hm0360_int_GPIO_Port GPIOE
#define camera_select_Pin GPIO_PIN_15
#define camera_select_GPIO_Port GPIOE
#define user_button_Pin GPIO_PIN_14
#define user_button_GPIO_Port GPIOB
#define hm0360_clksel_Pin GPIO_PIN_8
#define hm0360_clksel_GPIO_Port GPIOD
#define hm0360_xshut_Pin GPIO_PIN_9
#define hm0360_xshut_GPIO_Port GPIOD
#define hm0360_xsleep_Pin GPIO_PIN_10
#define hm0360_xsleep_GPIO_Port GPIOD
#define ldo_3v3_en_Pin GPIO_PIN_15
#define ldo_3v3_en_GPIO_Port GPIOA
#define ulpi_refclk_Pin GPIO_PIN_6
#define ulpi_refclk_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
