/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "tim.h"
#include <string.h>
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "scheduler/Scheduler.hpp"
#include "AppTasks.hpp"
#include "Sequencer.hpp"
#include "SequencerUI.hpp"
#include "Images.hpp"
#include "UartControl.hpp"
#include "drivers/Uart.hpp"
#include "drivers/Display.hpp"
#include "drivers/Keyboard.hpp"
#include "drivers/Buzzer.hpp"
#include "drivers/Synthesizer.hpp"
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
  MX_USART6_UART_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  
  // Тест UART сразу после инициализации (через драйвер)
  Uart::getInstance().printf("UART Test OK\n");
  /* USER CODE BEGIN 2 */
  // Инициализация планировщика
  Scheduler& scheduler = Scheduler::getInstance();
  scheduler.init();
  
  // Инициализация драйверов
  Display::getInstance().init();
  Keyboard::getInstance().init();
  Buzzer::getInstance().init();
  Synthesizer::getInstance().init();
  
  // Тестирование зуделки перед запуском шедулера
  Uart::getInstance().printf("=== BUZZER TEST START ===\n");
  
  // Настройка параметров зуделки
  Buzzer::getInstance().setGlobalVolume(8);  // Устанавливаем громкость 8 из 10
  Uart::getInstance().printf("Buzzer global volume set to 8\n");
  
  // Простой тест PWM - прямая настройка таймера
  Uart::getInstance().printf("Direct PWM test...\n");
  TIM1->PSC = 89;  // Предделитель
  TIM1->ARR = 100;  // Период
  TIM1->CCR1 = 500;  // Duty cycle 50%
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_Delay(100);
  TIM1->CCR1 = 0;  // Выключаем
  HAL_Delay(50);
  Uart::getInstance().printf("Direct PWM test completed\n");
  
  // Тестовая мелодия для проверки зуделки
  const Note testMelody[] = {
    Note(Buzzer::NOTE_C4, 200, 0),   // До 4-й октавы, 500мс
    Note(Buzzer::NOTE_E4, 200, 0),   // Ми 4-й октавы, 500мс  
    Note(Buzzer::NOTE_G4, 200, 0),   // Соль 4-й октавы, 500мс
    Note(Buzzer::NOTE_C5, 500, 0)   // До 5-й октавы, 1000мс
  };
  
  Melody testMelodyObj(testMelody, 4, 120);  // Темп 120 BPM
  Uart::getInstance().printf("Playing test melody...\n");
  Buzzer::getInstance().playMelody(testMelodyObj);
  
  // Ждем завершения мелодии
  uint32_t melodyStartTime = HAL_GetTick();
  while (Buzzer::getInstance().isPlaying() && (HAL_GetTick() - melodyStartTime < 1500)) {
    Buzzer::getInstance().update();
    HAL_Delay(10);
  }
  
  Uart::getInstance().printf("Test melody completed\n");
  // Тест синтезатора
  Uart::getInstance().printf("Testing synthesizer...\n");
  Synthesizer::getInstance().noteOn(0, 60, 64);  // MIDI note 60 (C4)
  HAL_Delay(1000);
  Synthesizer::getInstance().noteOff(0, 60);
  HAL_Delay(200);
  
  Synthesizer::getInstance().noteOn(0, 64, 64);  // MIDI note 64 (E4)
  HAL_Delay(1000);
  Synthesizer::getInstance().noteOff(0, 64);
  HAL_Delay(200);
  
  Synthesizer::getInstance().noteOn(0, 67, 64);  // MIDI note 67 (G4)
  HAL_Delay(1000);
  Synthesizer::getInstance().allNotesOff();
  
  Uart::getInstance().printf("Synthesizer test completed\n");
  Uart::getInstance().printf("=== BUZZER TEST END ===\n");
  
  // Инициализация компонентов секвенсора
  Sequencer::getInstance().init();
  MenuSystem::getInstance().init();
  SequencerRenderer::getInstance().init();
  Images::getInstance().init();
  
  // Отладочный вывод через драйвер UART
  Uart::getInstance().printf("STM32F4 System Starting...\n");
  Uart::getInstance().printf("Scheduler initialized\n");
  Uart::getInstance().printf("Display initialized\n");
  Uart::getInstance().printf("Keyboard initialized\n");
  Uart::getInstance().printf("Buzzer initialized\n");
  Uart::getInstance().printf("Synthesizer initialized\n");
  
  // Добавление задач
  scheduler.addTask(new KeyboardTask());
  scheduler.addTask(new DisplayTask());
  scheduler.addTask(new UartTask());
  scheduler.addTask(new BuzzerTask());
  scheduler.addTask(new SynthesizerTask());
  scheduler.addTask(new PianoTask());
  scheduler.addTask(new SequencerTask());
  scheduler.addTask(new UartControlTask());
  scheduler.addTask(new DebugTask());
  
  // Отладочный вывод информации о задачах
  Uart::getInstance().printf("=== SCHEDULER INITIALIZATION ===\n");
  Uart::getInstance().printf("Added %d tasks to scheduler\n", scheduler.getTaskCount());
  scheduler.printTaskInfo();
  Uart::getInstance().printf("=== END SCHEDULER INFO ===\n");
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // Запускаем планировщик задач
    scheduler.run();
    
    // Обрабатываем UART драйвер (неблокирующая отправка)
    Uart::getInstance().process();
    
    // Минимальная задержка для стабильности системы
    HAL_Delay(1);
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

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
