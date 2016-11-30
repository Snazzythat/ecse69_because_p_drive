 /* ******************************************************************************
  * File Name          : TIM.c
  * Description        : TIM 4 initiator and ISR call back handling
	* Author						 : Armen, Roman
	* Date							 : Oct 31st, 2016
  ******************************************************************************
  */
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#include "TIM.h"
#include "LED_thread.h"
#include "mouse_thread.h"
#include "keypad_thread.h"

int keypad_TIM_counter;
int segment_TIM_counter;
int mouse_TIM_counter;

TIM_HandleTypeDef TIM3_handle;
TIM_HandleTypeDef TIM4_handle;

// TIM 4 INIT
void Tim4Init()
{	
	__HAL_RCC_TIM4_CLK_ENABLE();
	
	// Desired rate = ClockFrequency /(prescaler * period)
	// Clock frequency is 168MHz Period and prescaler are in the range
	// Let Prescaler be 100 and the period be 840
	TIM4_handle.Instance = TIM4;
	TIM4_handle.Init.Prescaler					= 100;        
	TIM4_handle.Init.CounterMode				= TIM_COUNTERMODE_UP;     
	TIM4_handle.Init.Period							= 840;           
	TIM4_handle.Init.ClockDivision			= TIM_CLOCKDIVISION_DIV1;    
	
	HAL_TIM_Base_Init(&TIM4_handle);
	HAL_TIM_Base_Start_IT(&TIM4_handle);
	
	HAL_NVIC_SetPriority(TIM4_IRQn, 0, 1);
	HAL_NVIC_EnableIRQ(TIM4_IRQn);
}

// TIM 4 CALL BACK 
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{	
	if(htim->Instance == TIM4)
	{
		// 1000HZ for display
		// A signal is being sent to segment_display thread at 1000Hz (this cycle lasts for 1 second)
		// The signal 0x0005 is sent at 500Hz (0.5 seconds) to turn off segments if conditions apply.
		if(segment_TIM_counter++ >= 500)
		{
			osSignalSet(segment_thread_ID, 0x0005);
			if (segment_TIM_counter == 1000)
				segment_TIM_counter = 0;
		}
		
		// 500HZ for blinking
		else
		{
			osSignalSet(segment_thread_ID, 0x0004);
		}
		
		osSignalSet(mouse_thread_ID, 0x0069);
		
		// 100hz Keypad and temp ADC
		if (keypad_TIM_counter++ == 100)
		{
			osSignalSet(tid_Thread_ADCTemp, 0x0005);
			osSignalSet(tid_Thread_Keypad, 0x0007);
			osSignalSet(LED_thread_ID, 0x00000001);
			
			keypad_TIM_counter = 0;
		}

	}
}
