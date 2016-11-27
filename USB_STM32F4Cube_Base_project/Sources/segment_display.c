////////////////////////////////////////////////////////////////////////////////
//	File Name					: main.c
//	Description				: header file for adc_example.h
//	Author						: Roman Andoni, Armen Stepanians
//	Date							: Oct 1st, 2016
////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------Includes
#include <stm32f4xx_hal.h>
#include <supporting_functions.h>
#include <segment_display.h>
#include <temperature_thread.h>
#include <keypad_thread.h>
#include <TIM.h>
#include <math.h>

//--------------------------------------------------------------------------------------Defines 
osThreadDef(segment_thread, osPriorityNormal, 1,0);
osThreadId segment_thread_ID;

//Mode that keyboard will trigger (accelerometer-temp)
int selected_mode;

//Temperature coming from ADC
float filteredTemp;

//Angle coming from accelerometer.
float displayed_angle;

//Mutex for segment display in order to be able for threads to set and get values without deadlock
osMutexId segment_mutex;
osMutexDef(segment_mutex);

//--------------------------------------------------------------------------------------Config for 7 segemnt display 
//Config the 7 seg display decimal point
void configure_7_segment_DP(int MODE, float value, int ALARM, int* DP_ARRAY)
{
	if ( ALARM == ALARM_SET)
	{
		DP_ARRAY[3] = GPIO_PIN_RESET;
		DP_ARRAY[2] = GPIO_PIN_RESET;
		DP_ARRAY[1] = GPIO_PIN_RESET;
		DP_ARRAY[0] = GPIO_PIN_RESET;
	}
	else if ( MODE == ROLL_MODE ||  MODE == PITCH_MODE){
		if (((int)value)/100 != 0)
		{
			DP_ARRAY[3] = GPIO_PIN_RESET;
			DP_ARRAY[2] = GPIO_PIN_RESET;
			DP_ARRAY[1] = GPIO_PIN_RESET;
			DP_ARRAY[0] = GPIO_PIN_RESET;
		}		
		else if (((int)value)/10 != 0)
		{
			DP_ARRAY[3] = GPIO_PIN_RESET;
			DP_ARRAY[2] = GPIO_PIN_SET;
			DP_ARRAY[1] = GPIO_PIN_RESET;
			DP_ARRAY[0] = GPIO_PIN_RESET;
		}		
		else
		{
			DP_ARRAY[3] = GPIO_PIN_SET;
			DP_ARRAY[2] = GPIO_PIN_RESET;
			DP_ARRAY[1] = GPIO_PIN_RESET;
			DP_ARRAY[0] = GPIO_PIN_RESET;
		}		
	}
	else if( MODE == TEMPERATURE_MODE )
	{
		DP_ARRAY[3] = GPIO_PIN_RESET;
		DP_ARRAY[2] = GPIO_PIN_SET;
		DP_ARRAY[1] = GPIO_PIN_RESET;
		DP_ARRAY[0] = GPIO_PIN_RESET;
	}
}

//Config the 7 seg display decimal point
void configure_7_segment_unit(int MODE, int *U_ARRAY)
{
	if ( MODE == ROLL_MODE ||  MODE == PITCH_MODE)
		*U_ARRAY = o;
	else if( MODE == TEMPERATURE_MODE )
		*U_ARRAY = c;
}

//--------------------------------------------------------------------------------------INIT 7 segment selector pin init
// Inits Chosing Pins for eaqch segment.
void seven_segment_select_init(int DISPLAY_NUM) {
	
	GPIO_InitTypeDef GPIO_InitB;
	SEVEN_SEG_DISP_CLK_EN; 									
	
	// LED definition pins
	GPIO_InitB.Pin = DISPLAY_NUM; 	
	GPIO_InitB.Mode = GPIO_MODE_OUTPUT_PP;   			
	GPIO_InitB.Pull = GPIO_NOPULL;
	GPIO_InitB.Speed = GPIO_SPEED_FREQ_MEDIUM;		
	
	HAL_GPIO_Init(SEVEN_SEGMENT_DISPLAY_PORT, &GPIO_InitB);
	HAL_GPIO_WritePin(SEVEN_SEGMENT_DISPLAY_PORT, DISPLAY_NUM, GPIO_PIN_RESET);
}

//--------------------------------------------------------------------------------------Display selector 
//Does action on leds (Turns on if off and off if on)
void seven_segment_select(int DISPLAY_NUM)
{
	if(DISPLAY_NUM != NULL)
	{
		HAL_GPIO_TogglePin(SEVEN_SEGMENT_DISPLAY_PORT, DISPLAY_NUM);
	}
}

//--------------------------------------------------------------------------------------Display value  
void select_and_display_segment(int DISPLAY1, int DISPLAY2, int DECIMAL_SETTER, int SEG_VALUE)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, DECIMAL_SETTER); 
	seven_segment_select(DISPLAY1);
	seven_segment_select(DISPLAY2);
	run_segment_display(SEG_VALUE);
}

//--------------------------------------------------------------------------------------Getter for decimals for temperature 
//Getter for temperature sensor decimals for 7 segment display
void get_processor_temp(int *dec1, int *dec2, int *dec3, float temp_value)
{
	// XX.Y
	*dec1 = (int)(fmod(temp_value, 1.0) * 10);
	*dec2 = (int)(fmod(temp_value, 10.0) - fmod(temp_value, 1.0));
	*dec3 = (int) temp_value / 10;
}

//--------------------------------------------------------------------------------------Getter for decimals for piezo (NON USED HERE)
//Getter for piezo decimals for 7 segment display
void get_piezo_decs(int *dec1, int *dec2, int *dec3, int piezo_num)
{
	// XXX
	*dec1 = (int)(fmod(piezo_num, 10.0));
	*dec2 = (int)(fmod(piezo_num, 100.0)/10);
	*dec3 = (int) piezo_num / 100;
}

//--------------------------------------------------------------------------------------Getter for decimals for angle (accelerometer) 
//Getter for accelerometer angle decimals for 7 segment display
void get_angle_decs(int *dec1, int *dec2, int *dec3, float angle)
{
	// if XXX format, can use get_piezo_decs method
	if (((int)angle)/100 != 0)
		get_piezo_decs(dec1, dec2, dec3, angle);
	// if XX.Y format, can use get_processor_temp method
	else if (((int)angle)/10 != 0)
		get_processor_temp(dec1, dec2, dec3, angle);
	// if X.YY format
	else
	{
		*dec1 = (int)(fmod(angle, 0.1) * 100);
		*dec2 = (int)(fmod(angle, 1.0) - fmod(angle, 0.1)) * 10;
		*dec3 = (int) angle;
	}
}

//--------------------------------------------------------------------------------------ACCESSORS with mutex
//Setting mutually exclusive filtered temp value
void SevenSegment_SetDisplayValue_Temp(float temp)
{
	/* We have race conditions. Need to use mutexes to protect these global flags. */
	osMutexWait(segment_mutex, osWaitForever);
	filteredTemp = temp;
	osMutexRelease(segment_mutex);
}


//Getting mutually exclusive filtered temp value
float SevenSegment_GetDisplayValue_Temp(void)
{
	float temp;
	
	/* We have race conditions. Need to use mutexes to protect these global flags. */
	osMutexWait(segment_mutex, osWaitForever);
	temp = filteredTemp;
	osMutexRelease(segment_mutex);
	
	return temp;
}

// Setting mutually exclusive angle value
void SevenSegment_SetDisplayValue_Angle(float angle)
{
	/* We have race conditions. Need to use mutexes to protect these global flags. */
	osMutexWait(segment_mutex, osWaitForever);
	displayed_angle = angle;
	osMutexRelease(segment_mutex);
}

//--------------------------------------------------------------------------------------THRESHHOLD ALARM SETTER (BLINKER)
// set or reset ALARM
void AlarmSet(float temperature, int *ALARM, int signal)
{
	if (temperature > 35 && signal == 0x0005)
		*ALARM = ALARM_SET;
	else
		*ALARM = ALARM_RESET;
}

// Gettting mutually exclusive angle value
float SevenSegment_GetDisplayValue_Angle(void)
{
	float angle;
	
	/* We have race conditions. Need to use mutexes to protect these global flags. */
	osMutexWait(segment_mutex, osWaitForever);
	angle = displayed_angle;
	osMutexRelease(segment_mutex);
	
	return angle;
}

//--------------------------------------------------------------------------------------THREAD starter
void start_segment_thread(void *args)
{
	segment_thread_ID = osThreadCreate(osThread(segment_thread), args);
	
	if(segment_thread_ID == NULL)
	{
		printf("Segment thread failed to load");
	}
	else
	{
		printf("Segment thread loaded successfully!\n");
	}
}

//--------------------------------------------------------------------------------------THREAD
void segment_thread(void const *args) {
	
	seven_segment_init();
	seven_segment_select_init(DISPLAY_0);
	seven_segment_select_init(DISPLAY_1);
	seven_segment_select_init(DISPLAY_2);
	seven_segment_select_init(DISPLAY_3);
	
	int time_counter = 0;
	float display_value;
	osEvent ev;
	int ALARM;
	
	int U_ARRAY;
	int DP_ARRAY[4];
	
	while(1)
	{
			int dec3, dec2, dec1;	
			ev = osSignalWait(0, osWaitForever);
			if (selected_mode == TEMPERATURE_MODE)
			{
				display_value = SevenSegment_GetDisplayValue_Temp();
				AlarmSet(display_value, &ALARM, ev.value.signals);
				get_processor_temp(&dec1, &dec2, &dec3, display_value);
			}
			else if (selected_mode == ROLL_MODE)
			{
				display_value = SevenSegment_GetDisplayValue_Temp();
				AlarmSet(display_value, &ALARM, ev.value.signals);
				display_value  = SevenSegment_GetDisplayValue_Angle();
				get_angle_decs(&dec1, &dec2, &dec3, display_value);
			}
			else if (selected_mode == PITCH_MODE)
			{
				display_value = SevenSegment_GetDisplayValue_Temp();
				AlarmSet(display_value, &ALARM, ev.value.signals);
				display_value = SevenSegment_GetDisplayValue_Angle();
				get_angle_decs(&dec1, &dec2, &dec3, display_value);
			}
			
			configure_7_segment_DP(selected_mode, display_value, ALARM, DP_ARRAY);
			configure_7_segment_unit(selected_mode, &U_ARRAY);
			
			if(selected_mode == TEMPERATURE_MODE || selected_mode == ROLL_MODE || selected_mode == PITCH_MODE)
			{
			
				//Getting the temp value using mutex
				
				if(time_counter == 2)
					select_and_display_segment(NULL, DISPLAY_0, DP_ARRAY[0], (ALARM == ALARM_SET) ? OFF : U_ARRAY);
				if(time_counter == 4)
					select_and_display_segment(DISPLAY_0, DISPLAY_1, DP_ARRAY[1], (ALARM == ALARM_SET) ? OFF : dec1);
				if(time_counter == 6)
					select_and_display_segment(DISPLAY_1, DISPLAY_2, DP_ARRAY[2], (ALARM == ALARM_SET) ? OFF : dec2);
				if(time_counter == 8)
						select_and_display_segment(DISPLAY_2, DISPLAY_3, DP_ARRAY[3],  (ALARM == ALARM_SET) ? OFF : dec3);
				if(time_counter++ == 10) { // increments every 1 ms, used to count when 1 second has passed
					time_counter = 0;
					seven_segment_select(DISPLAY_3);
				}
			}
	}
}

//--------------------------------------------------------------------------------------7 segemnt display GPIO init
void seven_segment_init()
{
	GPIO_InitTypeDef GPIO_InitB;
	SEVEN_SEG_DISP_CLK_EN;
	
	GPIO_InitB.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_5| GPIO_PIN_7 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14;
	GPIO_InitB.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitB.Pull = GPIO_PULLUP;
	GPIO_InitB.Speed =  GPIO_SPEED_FREQ_MEDIUM;
	
	HAL_GPIO_Init(SEVEN_SEGMENT_DISPLAY_PORT, &GPIO_InitB);	
}

//--------------------------------------------------------------------------------------Display switch segment selector
void run_segment_display(int selected_number)
{
	// 7 SEGMENT DISPLAY PINOUT
	// A: PIN 14   --> PB0
	// B: PIN 16	 --> PB1
	// C:	PIN 13	 --> PB5
	// D:	PIN 3	 	 --> PB11
	// E:	PIN 5	 	 --> PB12
	// F:	PIN 11	 --> PB13
	// G: PIN 15	 --> PB14
	
	switch(selected_number) {
		
			case E:
				// When EQUAL display E for user feedback
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); 			//A segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET); 		//B	segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); 		//C segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); 			//D segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); 			//E segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); 			//F segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);			//G segment 
				break;
		
			case L:
				// When LOW display L for user feedback
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); 		//A segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET); 		//B	segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); 		//C segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); 			//D segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); 			//E segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); 			//F segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);		//G segment 
				break;
			
			case H:
				// When HIGH display H for user feedback
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); 		//A segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); 			//B	segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); 			//C segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET); 		//D segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); 			//E segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); 			//F segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);			//G segment 
				break;
			
			case OFF:
				// when -2 do not display anything
				//printf("Case %d", digit);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); 			//A segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET); 			//B	segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); 			//C segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET); 			//D segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); 			//E segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); 			//F segment
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);			//G segment 
				break;
			
			case c:
				// when -1 display c (degrees)
				//printf("Case %d", digit);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); 			
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET); 		
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); 		
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET); 		
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); 		
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); 			
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);			
				break;
			
			case o:
				// when -1 display c (degrees)
				//printf("Case %d", digit);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); 			
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); 		
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); 		
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET); 		
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); 		
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); 			
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);			
				break;
			
			case 0:
				
				//printf("Case %d", digit);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); 			
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); 			
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); 			
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); 			
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); 			
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); 			
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);		
				break;
			
			case 1:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);  
				break;

			case 2:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); 
				break;
			
			case 3:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); 	 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); 	 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); 	 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); 	
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); 	 
				break;
			
			case 4:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);  
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); 	 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);		 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);	 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);   			
				break;
			
			case 5:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); 	  
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);	  
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); 	 	
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); 		
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);  
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); 		
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); 		
				break;
			
			case 6:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); 	
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); 	
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); 	
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); 	
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); 	
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); 	
				break;

			case 7:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); 	
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); 	
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); 	
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); 
				break;
			
			case 8:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); 
				break;
			
			case 9:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);  
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);  
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);  
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); 	 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); 	 		
				break;
			
			case 10:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); 
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); 		
				break;
				
			default:
				break;
		}
}
