/**
  ******************************************************************************
  * @file           : segment_lcd.h
  * @brief          : 7-segment LCD driver.
  * @author         : MicroTechnics (microtechnics.ru)
  ******************************************************************************
  */
#ifndef SEG_LCD_H
#define SEG_LCD_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
/* Declarations and definitions ----------------------------------------------*/
#define DIGITS_NUM                                                          4
#define SEGMENTS_NUM                                                        7
#define SEGMENT_PIN_ACTIVE                                                  1
#define DIGIT_PIN_ACTIVE                                                    SEGMENT_PIN_ACTIVE
#define DIGIT_CHARACTERS_NUM                                                10
#define EXTRA_CHARACTERS_NUM                                                2
#define ASCII_NUMBER_FIRST_CODE                                             0x30
#define ASCII_NUMBER_LAST_CODE                                              0x39
#define ASCII_DOT_CODE                                                      0x2E
typedef enum {
  SEG_LCD_OK,
  SEG_LCD_ERROR
} SEG_LCD_Result;
typedef struct SEG_LCD_ExtraCharacter
{
  uint8_t asciiCode;
  uint8_t symbolsTableOffset;
} SEG_LCD_ExtraCharacter;
typedef struct McuPin
{
  GPIO_TypeDef *port;
  uint16_t pin;
} McuPin;
/* Functions -----------------------------------------------------------------*/
extern void SEG_LCD_Process();
extern SEG_LCD_Result SEG_LCD_WriteNumber(float number);
extern SEG_LCD_Result SEG_LCD_WriteString(char* str);
#endif // #ifndef SEG_LCD_H
