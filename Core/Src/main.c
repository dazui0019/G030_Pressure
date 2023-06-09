/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tiny_lcd.h"
#include "it_manager.h"
#include "printf.h"
#include "pressure.h"
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
Prs_HandleTypeDef Pressure;
extern uint8_t backlightval;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
Prs_HandleTypeDef prsList = {.length = MAXSIZE};
double currentPrs = 0.0;

/**
 * @brief 滴答数，通过startTick和endTick的差值（时间长短）来判断肌肉的紧张程度
 */
uint32_t startTick = 0;
uint32_t endTick = 0;

/**
 * @brief 计时的标志位
 */
uint8_t endFlag = RESET;
uint8_t startFlag = SET;

/**
 * @brief 计数的气压终值（气压达到该值时，停止计时并显示肌肉紧张程度）,终值需要添加负号(即，-endVal)
 */
uint8_t endVal = 30;
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

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_I2C2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
    HAL_TIM_Base_Start_IT(&htim1);
    HAL_TIM_Base_Start_IT(&htim3);
    init_print(&huart1); // 重定向printf()
    printf("%s\r\n", "printf retargeted!");
    pressure_init(&hadc1, &prsList);
    println("pressure init");
    lcd_init();
    lcd_print_frame(); // 打印UI框架
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      // Tim1 每10ms触发1次（相当于每10ms采样1次气压值）
      if(Timer_State.Timer1){
          currentPrs = get_pressure();
          // 在气压到达特定值时记录时间
          if(currentPrs < -0.5 && startFlag){
              startTick = HAL_GetTick();
              char str[10];
              bon_ftos(str, currentPrs, 1);
              printf("prs: %skPa\r\n", str);
              printf("startTick: %d\r\n", startTick);
              startFlag = RESET;
              endFlag = SET;
          }
          else if(currentPrs <= -endVal && endFlag){
              lcd_print_val(currentPrs);
              endFlag=RESET;
              endTick = HAL_GetTick();
              char str[10];
              bon_ftos(str, currentPrs, 1);
              printf("prs: %skPa\r\n", str);
              printf("endTick: %d\r\n", endTick);
              if((endTick - startTick) > 5000){ lcd_display_sta_b(TENSION); }
              else { lcd_display_sta_b(RELAX); }
          }
          else if((currentPrs > -0.3) && !startFlag) { // 气压值大于-0.3重新开始记录
              startFlag = SET;
              lcd_display_sta(READY); // 重置肌肉状态
          }
          Timer_State.Timer1 = RESET;
      }

      // tim3 每250ms触发1次，刷新LCD
      if(Timer_State.Timer3){
          HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin); // 闪烁LED
          lcd_print_val((currentPrs >= -0.2) ? 0:currentPrs);
//          lcd_print_val(currentPrs);
          Timer_State.Timer3 = RESET;
      }

      /**
       * @brief 按下按键进入该函数，若在两秒内再次按下按键，则变量endVal自增5.
       */
      if(GPIO_State.KEY1){
          HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

          // 刷新LCD上的endVal
          lcd_setCursor(0, 1);
          lcd_printf("Up:%2d", endVal);
          lcd_setCursor(4, 1);
          lcd_sendCmd(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON); // 显示光标

          uint32_t uiTick = HAL_GetTick(); // 用来检测按键超时
          GPIO_State.KEY1 = RESET;
          while(1){
              if((HAL_GetTick()-uiTick) > 2000) { break; } // 检测按键是否超时（2s），在该时间内按下按键会刷新uiTick的值
              else if(GPIO_State.KEY1 == SET){ // 按下按键
                  // 自增endVal
                  if(endVal>=90) { endVal = 30; } // 大于90后，重置为30.
                  else { endVal += 5; }
//                  endVal = (endVal+5) % 90;
                  // 刷新LCD上的endVal
                  lcd_setCursor(0, 1);
                  lcd_printf("Up:%2d", endVal);
                  lcd_setCursor(4, 1);
                  uiTick = HAL_GetTick(); // 两秒内按下按键，则刷新uiTick值。
                  GPIO_State.KEY1 = RESET;
              }
          }
          lcd_sendCmd(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
      }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
