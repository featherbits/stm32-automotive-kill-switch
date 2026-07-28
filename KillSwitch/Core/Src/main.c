/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "gpio.h"
#include "tim.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdint.h>
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

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// delay in microseconds before master relay power output gets disabled
const uint32_t delayBeforePoweringOff = 1000 * 1000 * 2;  // 2 seconds

const bool isPowerEnableSwitchMomentary = true;

bool powerOffTriggered = false;

void powerOffMCU(void)
{
    HAL_GPIO_WritePin(
        MCUPowerSignal_GPIO_Port, MCUPowerSignal_Pin, GPIO_PIN_RESET
    );
}

void startPowerOffSequence(void)
{
    if (powerOffTriggered) return;

    powerOffTriggered = true;

    // signal external system that we are powering off
    HAL_GPIO_WritePin(
        ShutdownSignal_GPIO_Port, ShutdownSignal_Pin, GPIO_PIN_SET
    );

    // start master relay power off countdown
    HAL_TIM_Base_Start_IT(&htim2);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
    if (htim->Instance == TIM2)
    {
        // disable master relay power output
        HAL_GPIO_WritePin(
            OutputPowerSignal_GPIO_Port, OutputPowerSignal_Pin, GPIO_PIN_RESET
        );

        powerOffMCU();
    }
}

void onKillSwitchOn(void) { startPowerOffSequence(); }

void onPowerSwitchOn(void)
{
    if (isPowerEnableSwitchMomentary)
    {
        startPowerOffSequence();
    }
}

void onPowerSwitchOff(void)
{
    // if master output power is already off, just turn off the MCU
    if (HAL_GPIO_ReadPin(OutputPowerSignal_GPIO_Port, OutputPowerSignal_Pin)
        == GPIO_PIN_RESET)
    {
        powerOffMCU();
    }
}

// since buttons are externally pulled-up,
// falling edge triggers button state - ON
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin)
    {
        case KillSwitchSense_Pin:
            onKillSwitchOn();
            break;
        case PowerSignalSense_Pin:
            onPowerSwitchOn();
            break;
    }
}

// since buttons are externally pulled-up,
// rising edge restores default button state - OFF
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin)
    {
        case PowerSignalSense_Pin:
            onPowerSwitchOff();
            break;
    }
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU
     * Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the
     * Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_TIM2_Init();
    /* USER CODE BEGIN 2 */

    __HAL_TIM_SET_COUNTER(&htim2, delayBeforePoweringOff);

    // allow LPF to settle before reading switch states
    HAL_Delay(30);

    // cannot enable master relay power output when kill switch is ON
    if (HAL_GPIO_ReadPin(KillSwitchSense_GPIO_Port, KillSwitchSense_Pin)
        == GPIO_PIN_RESET)
    {
        while (1)
        {
        }
    }

    // enable master relay power output
    HAL_GPIO_WritePin(
        OutputPowerSignal_GPIO_Port, OutputPowerSignal_Pin, GPIO_PIN_SET
    );

    if (isPowerEnableSwitchMomentary)
    {
        // on startup MCU takes over control of its PSU enable signal
        // so that power enable button can disconnect
        HAL_GPIO_WritePin(
            MCUPowerSignal_GPIO_Port, MCUPowerSignal_Pin, GPIO_PIN_SET
        );
    }

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
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

    __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_0);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV4;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType
        = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
    /* User can add his own implementation to report the HAL error return state
     */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line
       number, ex: printf("Wrong parameters value: file %s on line %d\r\n",
       file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
