////////////////////////////////////////////////////////////////////////////////
//	File Name					: temperature_thread.c
//	Description				: Threaded module resposible for temperature sensor ADC init and thread sensor polling. Gets interrupt signals from TIM3
//	Author						: Roman Andoni, Armen Stepanians
//	Date							: Oct 1st, 2016
////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------INCLUDES      
#include "temperature_thread.h"
#include "kalman_filter_funcs.h"
#include "segment_display.h"
#include "TIM.h"

//--------------------------------------------------------------------------------------GLOBALS
// Using temperature code from prev. lab but now with threading integrated
ADC_HandleTypeDef example_ADC_Handle;

void Thread_ADCTemp (void const *argument);

osThreadId tid_Thread_ADCTemp;                              // thread id
osThreadDef(Thread_ADCTemp, osPriorityNormal , 1, 0);				// thread name, priority, instances, stack size

extern kalmanstruct_c kc1;

//--------------------------------------------------------------------------------------ADC INITIALIZER
void init_ADC_temperature(void)
{
	ADC_InitTypeDef example_ADC_Init; 																	// definition of ADC1 initialiation struct
	ADC_ChannelConfTypeDef example_ADC_Channel;													// definition of ADC1 channel struct
	HAL_LockTypeDef example_ADC_Lock; 																	// define ADC1 locking object
	ADC_MultiModeTypeDef example_ADC_Mode; 															// define ADC1 mode struct
	
	/*  initialize ADC init struct */
	example_ADC_Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV6;					// ADC Clock frequency 42MHz (168/4)
	example_ADC_Init.Resolution = ADC_RESOLUTION_12B;										// 12 bit resolution, better but slower
	example_ADC_Init.DataAlign = ADC_DATAALIGN_RIGHT;										// align the 12 bits data at the right of the 32 bits words
	example_ADC_Init.ScanConvMode = DISABLE;														// single channel mode
	example_ADC_Init.EOCSelection = ADC_EOC_SEQ_CONV;										// perform ADC conversions without having to read all conversion data
	example_ADC_Init.ContinuousConvMode = DISABLE;											// single mode convertion
	example_ADC_Init.DMAContinuousRequests = DISABLE;										// single mode DMA request
	example_ADC_Init.NbrOfConversion = 1;																// one conversion
	example_ADC_Init.DiscontinuousConvMode = ENABLE;										// enable discountinuous mode
	example_ADC_Init.NbrOfDiscConversion = 0;														// one conversion
	example_ADC_Init.ExternalTrigConv = ADC_SOFTWARE_START;							// no external trigger
	example_ADC_Init.ExternalTrigConvEdge = ADC_SOFTWARE_START;					// no external trigger
	
	/* initialize ADC handle struct */
	example_ADC_Handle.Instance = EXAMPLE_ADC;
	example_ADC_Handle.Init = example_ADC_Init;
	example_ADC_Handle.NbrOfCurrentConversionRank = 1;
	example_ADC_Handle.Lock = example_ADC_Lock;
	example_ADC_Handle.State = 0;
	example_ADC_Handle.ErrorCode = HAL_ADC_ERROR_NONE;
	
	/* initialize ADC channel struct */
	example_ADC_Channel.Channel = EXAMPLE_ADC_CHANNEL;
	example_ADC_Channel.Rank = 1;																				// use to determine the rank in which this channel is sampled
	example_ADC_Channel.SamplingTime = ADC_SAMPLETIME_480CYCLES;				// time for the internal capacitor to charge. longuer means more accurate
	example_ADC_Channel.Offset = 0;
	
	/* initialize ADC mode struct */
	example_ADC_Mode.Mode = ADC_MODE_INDEPENDENT;														
	example_ADC_Mode.DMAAccessMode = ADC_DMAACCESSMODE_DISABLED;
	example_ADC_Mode.TwoSamplingDelay = ADC_TWOSAMPLINGDELAY_5CYCLES;
	
	EXAMPLE_ADC_CLK_EN;
	HAL_ADC_ConfigChannel(&example_ADC_Handle, &example_ADC_Channel);
	HAL_ADCEx_MultiModeConfigChannel(&example_ADC_Handle, &example_ADC_Mode);	/* configure mode */

	GPIO_InitTypeDef GPIO_InitDef; 								
	EXAMPLE_ADC_GPIO_PORT_CLK_EN; 									
	
	GPIO_InitDef.Pin = EXAMPLE_ADC_GPIO_PIN; 	
	GPIO_InitDef.Mode = GPIO_MODE_ANALOG;   			
	GPIO_InitDef.Pull = GPIO_PULLDOWN;
	GPIO_InitDef.Speed = GPIO_SPEED_FREQ_MEDIUM;		
	
	HAL_GPIO_Init(EXAMPLE_ADC_GPIO_PORT, &GPIO_InitDef);
}

//--------------------------------------------------------------------------------------VOLT TO TEMP CONVERTER
float volt_to_temperature(float voltage)
{
	voltage -= (float)0.760; 		// Subtract the reference voltage at 25°C
	voltage /= (float).0025; 		// Divide by slope 2.5mV
	voltage += (float)25.0; 			// Add the 25°C
	
	// voltage = degrees at the end (Celsius)
	return voltage;
}

//--------------------------------------------------------------------------------------ADC RAW VALUE GETTER
// Temperature ADC poller. Called from thread function to get the raw temperature value
float get_raw_temp_value_adc(void) {
	
	float val = 0;
	
	if(HAL_ADC_PollForConversion(&example_ADC_Handle, POLL_TIMEOUT) == HAL_OK)
	{
		val = HAL_ADC_GetValue(&example_ADC_Handle);
	}
	HAL_ADC_Stop(&example_ADC_Handle);
	
	//HAL_ADC_Stop(&example_ADC_Handle);
	return (val*3.0)/4096.0;
}


//--------------------------------------------------------------------------------------MAIN THREAD WORKER FUNCTION
//Worker function that belongs to Temperature thread
void Thread_ADCTemp (void const *argument) 
{
	init_ADC_temperature();

	kalmanstruct_c kc1;
	initKalman(0.001, 0.1, &kc1);
	
	//ADC related constants.
	//Initialize 2 temperature result array (for the purposes of Kalman filter parameter comparison)
	float ADC_VOLTS;
	float MEASURED_TEMPERATURE[MEAS_LEN];
	float FILTERED_TEMPERATURE1[MEAS_LEN];
	float TEMPERATURE = 25.0;
	
	//While loop
	while(1)
	{		
		// wait for 100hz interrupt coming from TIM3 --> BLOCKING HERE UNTIL SIGNAL FROM TIM RECEIVED
		osSignalWait(0x0005, osWaitForever);
		
		//Get the temperature values (raw) from the ADC
		HAL_ADC_Start(&example_ADC_Handle);

		ADC_VOLTS = get_raw_temp_value_adc();
		
		//Convert the temperature values using the helper function
		TEMPERATURE = volt_to_temperature(ADC_VOLTS);
		
		//Preparing input array for Kalman filter
		MEASURED_TEMPERATURE[0] = TEMPERATURE;
		
		//Setting for Kalman input and calling Kalman for out
		runKalman(MEASURED_TEMPERATURE, FILTERED_TEMPERATURE1, &kc1);
		
		// Finally assigning globally extern variable to the Kalman output temperature
		// Variable is located in segemnt display for display purposes
		filteredTemp = FILTERED_TEMPERATURE1[0];
		SevenSegment_SetDisplayValue_Temp(filteredTemp);
	}
}

//--------------------------------------------------------------------------------------THREAD STARTER
//Temperature Thread started
int start_ADC_temperature_thread(void *args) 
{
  tid_Thread_ADCTemp = osThreadCreate(osThread(Thread_ADCTemp), args);
	
	if (tid_Thread_ADCTemp == NULL)
		{                                       
			printf("Failed to create a thread!!!!");
		}	
	else
	{
		printf("Successfully started temperature polling thread\n!");
	}
  return(0);
}

