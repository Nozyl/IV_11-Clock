#include "main.h"
#include "segment_lcd.h"
#include "stdio.h"
#include "button.h"

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;               // Идентификатор RTC
TIM_HandleTypeDef htim10;             // Идентификатор для обработки кнопок

RTC_TimeTypeDef sTime = {0};          // Структура для хранения времени
RTC_DateTypeDef DateToUpdate = {0};   // Структура для хранения даты

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);        // Настройка тактирования мк
static void MX_GPIO_Init(void);       // Инициализация портов
static void MX_RTC_Init(void);        // Инициализация RTC
static void MX_TIM10_Init(void);      // Инициализация таймера кнопок

int main(void)
{
  HAL_Init();                                       // Сброс периферийных устройств
  SystemClock_Config();                             // Настройка системной тактовой частоты

  // Периферия
  MX_GPIO_Init();                                   // Настройка пинов
  MX_RTC_Init();                                    // Настройка часов
  MX_TIM10_Init();                                  // Настройка таймера для кнопок

  HAL_PWR_EnableBkUpAccess();														// Разблокируем доступ к резервной области


//КАЛИБРОВКА=============================================================
  //  HAL_RTCEx_SetSmoothCalib(&hrtc,
  //		  RTC_SMOOTHCALIB_PERIOD_32SEC,
  //		  RTC_SMOOTHCALIB_PLUSPULSES_RESET,
  //		  427);
        //136
//=======================================================================

  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);   // Получаем текущее время из RTC
  HAL_RTC_GetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN);

  HAL_TIM_Base_Start_IT(&htim10);                   // Запускаем таймер кнопок в режиме прерываний

  while (1)
  {
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);                     // Получаем текущее время из RTC
	HAL_RTC_GetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN);              // Получаем текущую дату из RTC

	BUTTON_Process();
	if (BUTTON_GetAction(BUTTON_SETTINGS) == BUTTON_SHORT_PRESS)        // КОРОТКОЕ НАЖАТИЕ: увеличение минут
		{
		 sTime.Minutes++;                                               // Увеличиваем минуты на 1
		 sTime.Seconds = 0;                                             // Сбрасываем секунды в 0
		 if(sTime.Minutes >=60)                                         // Если достигли 60 минут
	         {
			 sTime.Seconds = 0;                                         // Сбрасываем секунды в 0
	         sTime.Minutes = 0;											// Сбрасываем минуты в 0
	         }
	         HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);			// Записываем новое время в RTC
			 }

	if (BUTTON_GetAction(BUTTON_SETTINGS) == BUTTON_LONG_PRESS)         // ДЛИННОЕ НАЖАТИЕ: увеличение часов
	    {
	     sTime.Hours++;													// Увеличиваем часы на 1
	     if(sTime.Hours >=24)											// Если достигли 24 часов
	         {
	         sTime.Hours = 0;											// Сбрасываем часы в 0
	         }
	         HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);			// Записываем новое время в RTC
	    	 }
	         BUTTON_ResetActions();										// Сбрасываем флаги действий кнопок

	  // Обновление 7-сегментного индикатора
      SEG_LCD_Process(); 															// Динамическая индикация (опрос разрядов)
      HAL_Delay(1);																	// Небольшая задержка для стабильности (также земедляят перебор
      // Формирование строки для вывода на индикатор
      char str[DIGITS_NUM + 2];														// Буфер для строки (4 символа + точка + запас)

      // Мигание точки с периодом 2 секунды (проверка четности секунд)
      if(sTime.Seconds % 2 == 0)													// Четные секунды - показываем точку
      {
    	  snprintf(str, DIGITS_NUM +2, "%02d.%02d", sTime.Hours, sTime.Minutes );   // Формат: "ЧЧ.ММ" (например "12.30")
      }
      else																			// Нечетные секунды - без точки
      {
    	  snprintf(str, DIGITS_NUM +2, "%02d%02d", sTime.Hours, sTime.Minutes );
      }
      // Отправляем строку в драйвер индикатора
      SEG_LCD_WriteString(str);
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  // Включаем тактирование блока питания и доступ к резервной области
  __HAL_RCC_PWR_CLK_ENABLE();   													// Включаем тактирование питания
   // HAL_PWR_EnableBkUpAccess(); 													// Разблокируем домен резервного питания
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2); 					// Настраиваем осцилляторы
  __HAL_RCC_RTC_ENABLE();															// Включаем тактирование RTC

  // Настройка генераторов HSE и LSE
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;											// Внешний кварц 25 МГц
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;											// Внешний кварц 32.768 кГц для RTC
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;										// Включаем PLL
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;								// Источник PLL - HSE
  RCC_OscInitStruct.PLL.PLLM = 25;													// Делитель: 25 МГц / 25 = 1 МГц
  RCC_OscInitStruct.PLL.PLLN = 168;													// Множитель: 1 МГц * 168 = 168 МГц
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;										// Делитель для системной шины: 168/2 = 84 МГц
  RCC_OscInitStruct.PLL.PLLQ = 4;													// Делитель для USB/SDIO

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  // Настройка делителей для шин AHB/APB
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;							// Источник SYSCLK - PLL
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;								// HCLK = SYSCLK (84 МГц)
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;									// PCLK1 = HCLK/2 (42 МГц)
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;									// PCLK2 = HCLK (84 МГц)

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_RTC_Init(void)
{
  RTC_TimeTypeDef sTime = {0};														// Временная структура для начальной установки
  RTC_DateTypeDef sDate = {0};														// Временная структура для начальной установки

  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;											// 24-часовой формат
  hrtc.Init.AsynchPrediv = 127;														// Асинхронный делитель (для LSE)
  hrtc.Init.SynchPrediv = 255;														// Синхронный делитель
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;											// Отключаем выход RTC
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != 0x32F2)								// Проверяем маркер инициализации в backup-регистре
  {
    // Первый запуск - устанавливаем корректное время и дату
    sTime.Hours = 12;
    sTime.Minutes = 0;
    sTime.Seconds = 0;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

    sDate.WeekDay = RTC_WEEKDAY_THURSDAY;
    sDate.Month = RTC_MONTH_FEBRUARY;
    sDate.Date = 12;
    sDate.Year = 26;  // 2026 год
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    // Сохраняем маркер инициализации
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0x32F2);
  }


}

static void MX_TIM10_Init(void)
{
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 8400-1;														// 84 МГц / 8400 = 10 кГц
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 10;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // Включаем тактирование портов
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // Устанавливаем начальное состояние выходов (все выключены)
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15
                          |GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_RESET);

  // Настройка пина кнопки (PA2) как вход
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;												// Без подтяжки (если есть внешняя)
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Настройка пинов сегментов и разрядов на PORTB как выходы
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15
                          |GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;                                      // Двухтактный выход
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;									   // Низкая скорость (для индикаторов)
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // Настройка пинов сегментов на PORTA как выходы
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}




void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == htim10.Instance)										  // Если прерывание от таймера 10 - обрабатываем кнопки
  {
    BUTTON_TimerProcess();
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }

}
