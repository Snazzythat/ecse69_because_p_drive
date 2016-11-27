////////////////////////////////////////////////////////////////////////////////
//	File Name					: temperature_thread.h
//	Description				: header file for temperature thread file
//	Author						: Roman Andoni, Armen Stepanians
//	Date							: Oct 1st, 2016
////////////////////////////////////////////////////////////////////////////////

#ifndef _ADC_EXAMPLE
#define _ADC_EXAMPLE

#define POLL_TIMEOUT 1000

#define ALARM_RESET 0
#define ALARM_SET 1

#define EXAMPLE_ADC ADC1
#define EXAMPLE_ADC_CHANNEL ADC_CHANNEL_16
#define EXAMPLE_ADC_CLK_EN 	__HAL_RCC_ADC1_CLK_ENABLE()

#define EXAMPLE_ADC_GPIO_PORT GPIOA
#define EXAMPLE_ADC_GPIO_PIN GPIO_PIN_5 // Pin A5 is connected to the ADC1 Channel 5 input
#define EXAMPLE_ADC_GPIO_PORT_CLK_EN __HAL_RCC_GPIOA_CLK_ENABLE();

#include <cmsis_os.h>

//Temperature sensor ADC thread starter
extern int start_ADC_temperature_thread(void *args);

extern osThreadId tid_Thread_ADCTemp;

//Voltage to temp converter
float volt_to_temperature(float voltage);

//ADC raw value getter
float get_raw_temp_value_adc(void);

//Init for temperature sensor ADC
void init_ADC_temperature(void);

#endif
