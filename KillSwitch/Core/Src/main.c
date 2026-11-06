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
#include "stm32c0xx_hal_gpio.h"
#include "tim.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdint.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum PowerSignal
{
    ON = GPIO_PIN_SET,
    OFF = GPIO_PIN_RESET
} PowerSignal;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

// delay in microseconds until primary output is turned off
const uint32_t delayUntilShutDown = 1000 * 1000 * 2;  // 2 seconds

const bool isPowerEnableSwitchMomentary = true;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

bool powerOffTriggered = false;

void setMCUPower(PowerSignal state)
{
    HAL_GPIO_WritePin(
        PowerSignalMCU_GPIO_Port, PowerSignalMCU_Pin, (GPIO_PinState)state
    );
}

void setPrimaryOutput(PowerSignal state)
{
    HAL_GPIO_WritePin(
        PrimaryPowerEnable_GPIO_Port,
        PrimaryPowerEnable_Pin,
        (GPIO_PinState)state
    );
}

void setSecondaryOutput(PowerSignal state)
{
    HAL_GPIO_WritePin(
        SecondaryPowerEnable_GPIO_Port,
        SecondaryPowerEnable_Pin,
        (GPIO_PinState)state
    );
}

PowerSignal getKillSwitchSignal(void)
{
    return HAL_GPIO_ReadPin(KillSwitchSense_GPIO_Port, KillSwitchSense_Pin)
            == GPIO_PIN_SET
        ? OFF
        : ON;
}

void startPowerOffSequence(void)
{
    // already powering off, ignoring this call..
    if (powerOffTriggered) return;

    powerOffTriggered = true;

    // signal external system that we are powering off
    setSecondaryOutput(ON);

    // start power off countdown
    HAL_TIM_Base_Start_IT(&htim2);
}

void onShutdownPeriodElapsed(void)
{
    // turn off the primary output
    setPrimaryOutput(OFF);
    // and then power off the board
    setMCUPower(OFF);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
    // shutdown period elapsed
    if (htim->Instance == TIM2)
    {
        onShutdownPeriodElapsed();
    }
}

void onKillSwitchOn(void) { startPowerOffSequence(); }

void onPowerSwitchOff(void) { startPowerOffSequence(); }

// since buttons are externally pulled-up,
// falling edge triggers button state - ON
void HAL_GPIO_EXTI_Falling_Callback(uint16_t pin)
{
    switch (pin)
    {
        case KillSwitchSense_Pin:
            onKillSwitchOn();
            break;
    }
}

// since buttons are externally pulled-up,
// rising edge restores default button state - OFF
void HAL_GPIO_EXTI_Rising_Callback(uint16_t pin)
{
    switch (pin)
    {
        case PowerSignalExternalSense_Pin:
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
    // in order to block all interrupt handlers for switches
    // simply disable all interrupts
    __disable_irq();
    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_TIM2_Init();
    /* USER CODE BEGIN 2 */

    __HAL_TIM_SET_COUNTER(&htim2, delayUntilShutDown);

    // allow RC network to settle before reading switch states
    HAL_Delay(100);

    // cannot enable primary output while kill switch is ON
    // dry spin until it is turned OFF
    while (getKillSwitchSignal() == ON)
    {
    }

    // -------------------------------------------------------
    // at this point we are good to go
    // start power on sequence

    __enable_irq();

    // keep board powered by signal from MCU
    setMCUPower(ON);
    setPrimaryOutput(ON);

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
