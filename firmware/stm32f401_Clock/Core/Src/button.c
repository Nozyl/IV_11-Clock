/**
  ******************************************************************************
  * @file           : button.c
  * @brief          : Buttons driver
  * @author         : TQFP (for https://microtechnics.ru/user-blogs/)
  ******************************************************************************
  */



/* Includes ------------------------------------------------------------------*/

#include "button.h"



/* Declarations and definitions ----------------------------------------------*/

// Configuration

static Mcupin buttons[BUTTONS_NUM] = {{GPIOA, GPIO_PIN_2}};

// End of configuration

static uint16_t debounceCounter[BUTTONS_NUM];
static uint8_t waitButtonRelease[BUTTONS_NUM];
static uint16_t buttonPressCounter[BUTTONS_NUM];

static ButtonAction buttonActions[BUTTONS_NUM];
static ButtonState buttonState[BUTTONS_NUM];



/* Functions -----------------------------------------------------------------*/

/******************************************************************************/
void BUTTON_LowLevelManager()
{
  uint8_t currentStates[BUTTONS_NUM];
  
  for (uint8_t i = 0; i < BUTTONS_NUM; i++)
  {
    currentStates[i] = HAL_GPIO_ReadPin(buttons[i].port, buttons[i].pin);
    
    switch (buttonState[i])
    {
      case BUTTON_STARTING:
        if (currentStates[i] == GPIO_BUTTON_NOT_PRESSED)
        {
          buttonState[i] = BUTTON_NOT_PRESSED;
        } 
        break;
      
      case BUTTON_NOT_PRESSED:
        if (currentStates[i] == GPIO_BUTTON_PRESSED)
        {
          buttonState[i] = BUTTON_WAIT_DEBOUNCE;
          debounceCounter[i] = 0;
        }
        break;
        
      case BUTTON_WAIT_DEBOUNCE:
        if (debounceCounter[i] == DEBOUNCE_TIME_MS)
        {
          if (currentStates[i] == GPIO_BUTTON_PRESSED)
          {
            buttonState[i] = BUTTON_PRESSED;
          }
          else
          {
            buttonState[i] = BUTTON_NOT_PRESSED;
          }
        }
        break;
        
      case BUTTON_PRESSED:
        if (currentStates[i] == GPIO_BUTTON_NOT_PRESSED)
        {
          buttonState[i] = BUTTON_WAIT_DEBOUNCE;
          debounceCounter[i] = 0;
        } 
        break;
        
      default:
        break;
    }
  }
}



/******************************************************************************/
void BUTTON_HighLevelManager()
{
  for (uint8_t i = 0; i < BUTTONS_NUM; i++)
  {
    if (buttonActions[i] == BUTTON_NONE)
    {
      if (waitButtonRelease[i] == 0)
      {
        if (buttonState[i] == BUTTON_PRESSED)
        {
          waitButtonRelease[i] = 1;
        }
      }
      else
      {
        if (buttonState[i] == BUTTON_NOT_PRESSED)
        {
          waitButtonRelease[i] = 0;

          if (buttonPressCounter[i] >= BUTTONS_VERY_LONG_PRESS_MS)
          {
            buttonActions[i] = BUTTON_VERY_LONG_PRESS;
          }
          else
          {
            if (buttonPressCounter[i] >= BUTTONS_LONG_PRESS_MS)
            {
              buttonActions[i] = BUTTON_LONG_PRESS;
            }
            else
            {
              buttonActions[i] = BUTTON_SHORT_PRESS;
            }
          }
        }
      }
    }
  }
}



/******************************************************************************/
void BUTTON_Process()
{
  BUTTON_LowLevelManager();
  BUTTON_HighLevelManager();
}



/******************************************************************************/
void BUTTON_TimerProcess()
{
  for (uint8_t i = 0; i < BUTTONS_NUM; i++)
  {
    if (debounceCounter[i] < DEBOUNCE_TIME_MS)
    {
      debounceCounter[i]++;
    }

    if (waitButtonRelease[i] == 1)
    {
      buttonPressCounter[i]++;
    }
    else
    {
      buttonPressCounter[i] = 0;
    }
  }
}



/******************************************************************************/
ButtonAction BUTTON_GetAction(uint8_t index)
{
  return buttonActions[index];
}



/******************************************************************************/
void BUTTON_ResetActions()
{
  for (uint8_t i = 0; i < BUTTONS_NUM; i++)
  {
    buttonActions[i] = BUTTON_NONE;
  }
}



/******************************************************************************/
void BUTTON_Init()
{
  for (uint8_t i = 0; i < BUTTONS_NUM; i++)
  {
    debounceCounter[i] = 0;
    waitButtonRelease[i] = 0;
    buttonPressCounter[i] = 0;
    buttonActions[i] = BUTTON_NONE;
    buttonState[i] = BUTTON_STARTING;
  }
}



/******************************************************************************/
