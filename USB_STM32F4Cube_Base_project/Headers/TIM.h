/**
  ******************************************************************************
  * @file    TIM.h
  * @author  Armen Stepanians, Roman Andoni
  * @version V1.0.0
  * @date    Mar.4th, 2016
  * @brief   Header file of TIM.c 
  ******************************************************************************
*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#include <cmsis_os.h>

//#define OVERHEAT 60

//for display: decide the display mode
extern int mode;		// 1-accelerometer; 2 - temperature
extern int submode;	// 0(*) roll, 1(#) pitch
extern osThreadId LED_thread_ID;
extern osThreadId segment_thread_ID;
extern osThreadId tid_Thread_ADCTemp;
extern osThreadId tid_Thread_Keypad;

extern void Tim4Init();	

/**
  * @brief  Initialize the TIM
  * @param  TIM_Handle(TIM_HandleTypeDef)
  * @retval None
  */
void TimInit(TIM_HandleTypeDef *TIM_handle);

/**
  * @brief  Period Elapased Call Back
  * @param  TIM_Handle(TIM_HandleTypeDef)
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

