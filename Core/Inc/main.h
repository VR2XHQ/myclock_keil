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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
extern uint32_t T_10ms;		//Variable for 10ms count
extern uint32_t T_100ms;	//Variable for 100ms count
extern uint32_t T_1000ms;	//Variable for 1000ms count
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
#define FLASH_CS_Pin GPIO_PIN_13
#define FLASH_CS_GPIO_Port GPIOC
#define EXT_IO1_Pin GPIO_PIN_14
#define EXT_IO1_GPIO_Port GPIOC
#define EXT_IO2_Pin GPIO_PIN_15
#define EXT_IO2_GPIO_Port GPIOC
#define OSC_IN_Pin GPIO_PIN_0
#define OSC_IN_GPIO_Port GPIOD
#define OSC_OUT_Pin GPIO_PIN_1
#define OSC_OUT_GPIO_Port GPIOD
#define S5_Pin GPIO_PIN_0
#define S5_GPIO_Port GPIOA
#define LCD_RS_Pin GPIO_PIN_1
#define LCD_RS_GPIO_Port GPIOA
#define LCD_CS_Pin GPIO_PIN_2
#define LCD_CS_GPIO_Port GPIOA
#define IC_Pin GPIO_PIN_3
#define IC_GPIO_Port GPIOA
#define ADC_Pin GPIO_PIN_4
#define ADC_GPIO_Port GPIOA
#define SPI_SCK_Pin GPIO_PIN_5
#define SPI_SCK_GPIO_Port GPIOA
#define SPI_MISO_Pin GPIO_PIN_6
#define SPI_MISO_GPIO_Port GPIOA
#define SPI_MOSI_Pin GPIO_PIN_7
#define SPI_MOSI_GPIO_Port GPIOA
#define PTT_Pin GPIO_PIN_0
#define PTT_GPIO_Port GPIOB
#define CW_Pin GPIO_PIN_1
#define CW_GPIO_Port GPIOB
#define PWR_CTRL_Pin GPIO_PIN_2
#define PWR_CTRL_GPIO_Port GPIOB
#define I2C_SCL_Pin GPIO_PIN_10
#define I2C_SCL_GPIO_Port GPIOB
#define I2C_SDA_Pin GPIO_PIN_11
#define I2C_SDA_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_12
#define LED2_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_13
#define LED1_GPIO_Port GPIOB
#define _18B20_PIN_Pin GPIO_PIN_14
#define _18B20_PIN_GPIO_Port GPIOB
#define _18B20_VDD_Pin GPIO_PIN_15
#define _18B20_VDD_GPIO_Port GPIOB
#define SP_Pin GPIO_PIN_8
#define SP_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define S4_Pin GPIO_PIN_15
#define S4_GPIO_Port GPIOA
#define S3_Pin GPIO_PIN_3
#define S3_GPIO_Port GPIOB
#define Up_Pin GPIO_PIN_4
#define Up_GPIO_Port GPIOB
#define Down_Pin GPIO_PIN_5
#define Down_GPIO_Port GPIOB
#define S2_Pin GPIO_PIN_6
#define S2_GPIO_Port GPIOB
#define S1_Pin GPIO_PIN_7
#define S1_GPIO_Port GPIOB
#define LCD_RESET_Pin GPIO_PIN_8
#define LCD_RESET_GPIO_Port GPIOB
#define LCD_BK_Pin GPIO_PIN_9
#define LCD_BK_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define		TRUE            1
#define		FALSE           0
#define		enable			0x01
#define		disable			0x00
// Macro imported from DS18B20 project
#define LCDBK_ON()		HAL_GPIO_WritePin(LCD_BK_GPIO_Port, LCD_BK_Pin, GPIO_PIN_SET)
#define LCDBK_OFF()		HAL_GPIO_WritePin(LCD_BK_GPIO_Port, LCD_BK_Pin, GPIO_PIN_RESET)
#define DS18B20_ON()	HAL_GPIO_WritePin(_18B20_VDD_GPIO_Port, _18B20_VDD_Pin, GPIO_PIN_SET)
#define DS18B20_OFF()	HAL_GPIO_WritePin(_18B20_VDD_GPIO_Port, _18B20_VDD_Pin, GPIO_PIN_RESET)
// Macro for switch on/off for LCD or external device
#define Pwr_ON()	HAL_GPIO_WritePin(PWR_CTRL_GPIO_Port, PWR_CTRL_Pin, GPIO_PIN_SET)
#define Pwr_OFF()	HAL_GPIO_WritePin(PWR_CTRL_GPIO_Port, PWR_CTRL_Pin, GPIO_PIN_RESET)
// Macro for LED on/off
#define LED1_ON()		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET)
#define LED1_OFF()		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET)
#define LED2_ON()		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET)
#define LED2_OFF()		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET)
#define Blink_LED1()	LED1_ON();HAL_Delay(10);LED1_OFF();
#define Blink_LED2()	LED2_ON();HAL_Delay(10);LED2_OFF();
// Macro for key input
#define Read_F1()		HAL_GPIO_ReadPin(S1_GPIO_Port, S1_Pin)
#define Read_F2()		HAL_GPIO_ReadPin(S2_GPIO_Port, S2_Pin)
#define Read_F3()		HAL_GPIO_ReadPin(S3_GPIO_Port, S3_Pin)
#define Read_F4()		HAL_GPIO_ReadPin(S4_GPIO_Port, S4_Pin)
#define Read_PwrKey()	HAL_GPIO_ReadPin(S5_GPIO_Port, S5_Pin)
#define Read_R()		HAL_GPIO_ReadPin(Down_GPIO_Port, Down_Pin)
#define Read_L()		HAL_GPIO_ReadPin(Up_GPIO_Port, Up_Pin)
// Macro for Speaker on/off
#define SP_ON()		HAL_TIM_OC_Start(&htim1,TIM_CHANNEL_1)
#define SP_OFF()	HAL_TIM_OC_Stop(&htim1,TIM_CHANNEL_1)
// Others
#define Read_RTC_int()	HAL_GPIO_ReadPin(EXT_IO1_GPIO_Port, EXT_IO1_Pin)  //RTC chip pin 7, INT output
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
