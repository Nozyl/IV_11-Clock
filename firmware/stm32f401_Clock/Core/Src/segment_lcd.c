/**
  ******************************************************************************
  * @file           : segment_lcd.c
  * @brief          : 7-segment LCD driver.
  * @author         : MicroTechnics (microtechnics.ru)
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "segment_lcd.h"
#include <stdio.h>
/* Declarations and definitions ----------------------------------------------*/
static McuPin digitPins[DIGITS_NUM] = { {GPIOB, GPIO_PIN_3}, {GPIOB, GPIO_PIN_4},
                                        {GPIOB, GPIO_PIN_5}, {GPIOB, GPIO_PIN_7} };
static McuPin segmentPins[SEGMENTS_NUM] = { {GPIOA, GPIO_PIN_11}, {GPIOA, GPIO_PIN_10},
                                            {GPIOA, GPIO_PIN_9}, {GPIOA, GPIO_PIN_8},
                                            {GPIOB, GPIO_PIN_15}, {GPIOB, GPIO_PIN_14},
                                            {GPIOB, GPIO_PIN_13} };
static McuPin dotPin = {GPIOB, GPIO_PIN_12};
static uint8_t charactersTable[DIGIT_CHARACTERS_NUM + EXTRA_CHARACTERS_NUM] =
               {0x77, 0x24, 0x5D, 0x6D, 0x2E, 0x6B, 0x7B, 0x25, 0x7F, 0x6F, 0x40, 0x00};
static uint8_t currentCharacters[DIGITS_NUM] = {0x00, 0x00, 0x00, 0x00};
static uint8_t currentDots[DIGITS_NUM] = {0, 0, 0, 0};
static uint8_t currentDigitIndex = 0;
/* Functions -----------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
SEG_LCD_Result SEG_LCD_WriteString(char* str)
{
  uint8_t currentDigitIndex = 0;
  for (uint8_t i = 0; i < DIGITS_NUM; i++)
  {
    currentCharacters[i] = 0x00;
    currentDots[i] = 0;
  }
  while(*str != '\0')
  {
    if (*str == ASCII_DOT_CODE)
    {

      {
        currentDots[currentDigitIndex] = 1;
      }
    }
    else
    {
      if ((*str >= ASCII_NUMBER_FIRST_CODE) && (*str <= ASCII_NUMBER_LAST_CODE))
      {
        uint8_t currentCharacterIndex = (*str - ASCII_NUMBER_FIRST_CODE);
        currentCharacters[currentDigitIndex] = charactersTable[currentCharacterIndex];
        currentDigitIndex++;
      }
      else
      {
        uint8_t found = 0;

        if (found == 0)
        {
          return SEG_LCD_ERROR;
        }
      }
    }
    if (currentDigitIndex == DIGITS_NUM)
    {
      break;
    }
    str++;
  }
  if (currentDigitIndex < DIGITS_NUM)
  {
    for (int8_t i = currentDigitIndex - 1; i >= 0; i--)
    {
      currentCharacters[i + (DIGITS_NUM - currentDigitIndex)] = currentCharacters[i];
    }
    for (uint8_t i = 0; i < (DIGITS_NUM - currentDigitIndex); i++)
    {
      currentCharacters[i] = 0x00;
    }
  }

  return SEG_LCD_OK;
}
/*----------------------------------------------------------------------------*/
static void SetOutput(McuPin output, uint8_t state)
{
  HAL_GPIO_WritePin(output.port, output.pin, (GPIO_PinState)state);
}
/*----------------------------------------------------------------------------*/
static void SetSegmentPins(uint8_t characterCode)
{
  for (uint8_t i = 0; i < SEGMENTS_NUM; i++)
  {
    uint8_t bit = (characterCode >> i) & 0x01;
    if (bit == 1)
    {
      SetOutput(segmentPins[i], SEGMENT_PIN_ACTIVE);
    }
    else
    {
      SetOutput(segmentPins[i], !SEGMENT_PIN_ACTIVE);
    }
  }
}
/*----------------------------------------------------------------------------*/
void SEG_LCD_Process() //Функция перебора индикаторов
{
    for (uint8_t i = 0; i < DIGITS_NUM; i++)
    {
        SetOutput(digitPins[i], !DIGIT_PIN_ACTIVE);  // Выключаем все разряды
    }

    SetSegmentPins(currentCharacters[currentDigitIndex]);  // Устанавливаем сегменты

    if (currentDots[currentDigitIndex] == 1)
    {
        SetOutput(dotPin, SEGMENT_PIN_ACTIVE);
    }
    else
    {
        SetOutput(dotPin, !SEGMENT_PIN_ACTIVE);
    }

    SetOutput(digitPins[currentDigitIndex], DIGIT_PIN_ACTIVE);  // Включаем текущий

    currentDigitIndex++;
    if (currentDigitIndex == DIGITS_NUM) currentDigitIndex = 0;
}
/*----------------------------------------------------------------------------*/
