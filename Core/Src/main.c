/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "stdio.h"
//#include <stdlib.h>
#include "SD3078_rtc_task.h"
#include "ds18b20_task.h"
#include "Thermometer.h"
//#include "Lcd_Driver.h"
#include "GUI.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

//void DS18B20_function_set(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  SCB->VTOR = 0x08005000U;
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C2_Init();
  MX_SPI1_Init();
  MX_TIM1_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  MyClock_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
	  /* Refresh display of time every second */
	  uint32_t old_second;
	  if(T_1000ms > old_second){
		Show_time(m.STATUS,0);
		old_second = T_1000ms;
	  }

	  /* Detect alarm and how to handle */
	  if(Read_RTC_int()==FALSE){
		  Alarm_handle();
	  }

	  /*  Detect key press for menu access and action */
	  if(Read_F1() || Read_F2() || Read_R() || Read_L() || Read_F3() || Read_F4() || Read_PwrKey()){
		  Blink_LED2();  //Blink LED2 while any key press

		  /* Exit to root menu from first level of sub-menu */
		  if(Read_F1()){
			  HAL_Delay(50);  //Avoid fake key press
	 		  if(Read_F1()==TRUE && m.MODE!=0 && m.STATUS!=9){  //Access allow under certain condition
	 			  Reset_menu();
	 			  HAL_Delay(300);  //Delay to avoid further key press trigger launch of utc action
	 		  }
		  }

		  /* Enter mode sub-menu when press F4 key or wheel R */
	  	  if(Read_F4() || Read_R()){
	  		  HAL_Delay(50);  //Avoid fake key press
	  		  if((Read_F4()==TRUE || Read_R()==TRUE) && m.STATUS==0 && m.LCD==TRUE){  //Access allow under certain condition
	  			  m.MODE++;  //Move pointer by one direction
		  		  if(m.MODE<=5){  //Sub-menu pointer boundary  m.MODE>0 &&
		  			  Draw_menu(m.MODE);  //Draw sub-menu according to MODE
		  		  }
		  		  else{
		  			  Reset_menu();  //Exit sub-menu when reach boundary
		  		  }
		  	  }
	  	  }

/******************************* root of the menu bar **********************************/
	  	  /* MODE=0, Root of the menu bar */
	  	  if(m.MODE==0){
	  		  /* Toggle LCD on/off for power saving */
 			  if(Read_PwrKey()==TRUE && m.STATUS==0){
 				  Display_on_off();
 				  HAL_Delay(300);  //Delay to avoid further key press trigger the shut down call
  			  }
 			  if(Read_L()==TRUE && m.STATUS==0){
 				  m.AL_mode^=0x01;
 				  ClearAlarm(m.AL_mode,m.IM);
 				  Draw_menu(0);
 			  }

 			  /* Root menu function access only when LCD is on */
 			  if(m.LCD==TRUE && m.STATUS==0){
 				  if(Read_F1()==TRUE){
 					  m.UTC ^= 0x01;  //Toggle local and UTC time
 					  Show_time(9,0);  //Set status=9 for time display update once
 				  }
 				  if(Read_F2()==TRUE){  //Access DS18B20 function set
 					  DS18B20_function_set();
 					  Show_time(9,0);  //Set status=9 for time display update once
 					  Reset_menu();  //Use of reset menu is depended on the function call
 				  }
 				  if(Read_F3()==TRUE){
 					  Draw_menu(10);  //Show Timer sub-menu
 					  m.STATUS=9;  //Flow control for Timer sub-menu access
 					  HAL_Delay(300);  //Delay to avoid further key press trigger sub-menu function directly
 				  }
 			  }

 			  /* Timer sub-menu access */
 	 		  if((Read_F1() || Read_F2() || Read_F3()) && m.STATUS==9){
 	 			  HAL_Delay(50);  //Avoid fake key press
 	 			  if(Read_F1()==TRUE){
 	 				  Reset_menu();  //Return to root menu
 	 				  HAL_Delay(300);  //Delay to avoid further key press trigger launch of utc action
 	 			  }
	 	 			  if(Read_F2()==TRUE){
	 	 				  Count_down();  //Call count down function
	 	 			  }
	 	 			  if(Read_F3()==TRUE){
	 	 				  Stop_watch();  //Call stop watch function
 	 			  }
 	 		  }
 		  }
/************************** end of root menu selection ****************************/

/****************************** entry of mode sub-menu ****************************/
 		  /*MODE=1, Show RTC register information and make configuration */
 		  if(m.MODE==1){
 	  		  if(Read_F2()==TRUE){
 	  			  Toggle_AMPM();
 	  		  }
	  		  if(Read_F3()==TRUE){
	  			  TnB_adjust_conf();  //Set SD3078 RTC temperature and battery configuration
	  		  }
	  	  }

	  	  /*  MODE=2, Set hourly repeat or one time alarm */
  		  if(m.MODE==2){
  			  if(Read_F2()==TRUE){
  				  Set_repeat_alarm();  //Set repeat alarm
  			  }
  			  if(Read_F3()==TRUE){
  				  Time_setting(m.MODE);  //Set one time alarm
  			  }
  		  }

  		  /* MODE=3, Set which weekday, IM and weekly alarm */
  		  if(m.MODE==3){
  			  if(Read_F2()==TRUE){
  				  Weekday_setting();  //Enable which weekday for alarm
  			  }
  			  if(Read_F3()==TRUE){
  				  Time_setting(m.MODE);  //Set weekday alarm
  			  }
  		  }

  		  /* MODE=4, Set current date&time to RTC, plus sub-menu function */
  		  if(m.MODE==4){
  			  if(Read_F2()==TRUE && m.STATUS==0){
  				  m.STATUS=9;  //Flow control for next sub-menu access
  				  Draw_menu(41);  //Show next sub-menu
  				  HAL_Delay(300);  //Delay to avoid further key press trigger sub-menu function directly
  			  }
  			  if((Read_F1() || Read_F2() || Read_F3()) && m.STATUS==9){
  				  HAL_Delay(50);  //Avoid fake key press
  				  if(Read_F1()==TRUE){  //Exit to previous sub-menu
  					  m.STATUS=0;
  					  Draw_menu(4);  //Redraw previous sub-menu
  				  }
		  		  if(Read_F2()==TRUE){
		  			  Draw_menu(42);  //Show next sub-menu
	  				  Show_RTC_info();  //Show SD3078 RTC information
	  				  Draw_menu(41);  //Redraw previous sub-menu
		  		  }
	  			  if(Read_F3()==TRUE){
	  				  Toggle_TS();  //Select temperature source to display
	  			  }
  			  }
  			  if(Read_F3()==TRUE && m.STATUS==0){
  				  Time_setting(m.MODE);  //Set current date&time
  			  }
  		  }

  		  /* MODE=5, Frequency Out and others */
  		  if(m.MODE==5){
  			  if(Read_F2()==TRUE && m.STATUS==0){
  				  m.STATUS=9;  //Flow control for next sub-menu access
  				  Draw_menu(51);  //Show next sub-menu
  				  HAL_Delay(300);  //Delay to avoid further key press trigger sub-menu function directly
  			  }
			  if((Read_F1() || Read_F2() || Read_F3()) && m.STATUS==9){
				  HAL_Delay(50);  //Avoid fake key press
  				  if(Read_F1()==TRUE){  //Exit to previous sub-menu
  					  m.STATUS=0;
  					  Draw_menu(5);  //Redraw previous sub-menu
  				  }
	  			  if(Read_F2()==TRUE){  //Reserve function call
	  				  /* Example */
//	  				  DS18B20_function_set();
//					  Draw_menu(51);  //Redraw next sub-menu
//	  				  Draw_title(enable);  //Redraw title if needed
	  			  }
	  			  if(Read_F3()==TRUE){  //Reserve function call
	  				  /* Example */
//	  				  Alarm_stop();
//					  Draw_menu(51);  //Redraw next sub-menu
//	  				  Draw_title(enable);  //Redraw title if needed
	  			  }
			  }
  			  if(Read_F3()==TRUE && m.STATUS==0){
  				  Freq_out();  //Execute Freq_Out function
  			  }
  		  }
/****************** end the loop for mode menu and relevant execution ******************/
	  }  //End if key detection

	  /* soft power switch */
	  if(Read_PwrKey())	//If F5 PwrKey press
	  {
		  HAL_Delay(1000);	//If over 1 second pressed
		  if(Read_PwrKey()==TRUE)  //Check again if PwrKey press, then MCU goto standby mode
		  {
			  __HAL_RCC_PWR_CLK_ENABLE();							//Begin low power standby mode procedure
			  if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)			//Clear standby mode flag
				  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
			  HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);				//Set PA0 as wake-up pin

			  /* when switch off, LED2 turn on and SP beep 0.2 second */
			  Blink_LED2();Beep(1);

			  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);	//Switch off external power supply
			  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);					//Clear all relevant wakeup flag
			  HAL_PWR_EnterSTANDBYMode();							//Enter standby mode
		  }
	  }

  }  // while(1) end

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
