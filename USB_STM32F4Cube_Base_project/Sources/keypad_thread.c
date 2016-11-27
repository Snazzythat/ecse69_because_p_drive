////////////////////////////////////////////////////////////////////////////////
//	File Name					: keypad_thread.c
//	Description				: Threaded module resposible for getting values from the keypad
//	Author						: Roman Andoni, Armen Stepanians
//	Date							: Oct 1st, 2016
////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------INCLUDES
#include <stdbool.h>
#include "TIM.h"
#include "keypad_thread.h"
#include "segment_display.h"
//--------------------------------------------------------------------------------------GLOBALS

void Thread_Keypad (void const *argument);
osThreadId tid_Thread_Keypad;                              // thread id
osThreadDef(Thread_Keypad, osPriorityNormal, 1, 0);	// thread name, priority, instances, stack size

GPIO_InitTypeDef GPIO_InitCOLS;
GPIO_InitTypeDef GPIO_InitROWS;

// Struct holding keyboard values
typedef struct keypad_config
{
	bool READ;
	bool DISPLAY;
	int mode;	
	int number;
	
}keypad_config;

//--------------------------------------------------------------------------------------KEYPAD FUNCTIONS

//Initiating keypad config struct
void init_keypad_config(keypad_config *kbc)
{
	kbc->READ = false;
	kbc->DISPLAY = false;
}

// Getting keyboard value ONCE and filtering out the pressed repetitions
void get_kb_value(int keypad_pressed_num, keypad_config* kc)
{
	if (keypad_pressed_num == NON_POLLABLE)
	{
		kc->READ = true;
		kc->DISPLAY = false;
	}
	if (keypad_pressed_num != NON_POLLABLE && kc->READ)
	{
		kc->READ = false;
		kc->DISPLAY = true;
		kc->number = keypad_pressed_num;
	}
	else kc->number = NON_POLLABLE;
}

// Row initiator
void init_rows(void)
{

	//ROW INIT as pull up (high)
	GPIO_InitROWS.Pin = ROW1 | ROW2 | ROW3 | ROW4; 
	GPIO_InitROWS.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitROWS.Pull = GPIO_PULLUP;
	GPIO_InitROWS.Speed =  GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(ROW_GPIO, &GPIO_InitROWS);
	

	//COLUMNT INIT as no pull
	GPIO_InitCOLS.Pin = COLUMN1 | COLUMN2 | COLUMN3 ;
	GPIO_InitCOLS.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitCOLS.Pull = GPIO_NOPULL;
	GPIO_InitCOLS.Speed =  GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(COL_GPIO, &GPIO_InitCOLS);
	NUM_PAD_PORT_CLK_EN
}

// Column initiator
void init_columns(void)
{

	//ROW INIT as no pull
	GPIO_InitROWS.Pin = ROW1 | ROW2 | ROW3 | ROW4; 
	GPIO_InitROWS.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitROWS.Pull = GPIO_NOPULL;
	GPIO_InitROWS.Speed =  GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(ROW_GPIO, &GPIO_InitROWS);
	

	//COL INIT as pull up (high)
	GPIO_InitCOLS.Pin = COLUMN1 | COLUMN2 | COLUMN3 ;
	GPIO_InitCOLS.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitCOLS.Pull = GPIO_PULLUP;
	GPIO_InitCOLS.Speed =  GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(COL_GPIO, &GPIO_InitCOLS);
	NUM_PAD_PORT_CLK_EN
}

// Column getter (read pin and get 0 at the pressed column)
int get_col(void)
{
	// Initialize pins to allow for rows as input
	init_columns();
	
	//Active low to get the column
	if(!HAL_GPIO_ReadPin(COL_GPIO, COLUMN1))
	{
		return 0;
	}
	else if(!HAL_GPIO_ReadPin(COL_GPIO, COLUMN2))
	{
		return 1;
	}
	else if(!HAL_GPIO_ReadPin(COL_GPIO, COLUMN3))
	{
		return 2;
	}
	else return NON_POLLABLE;
}

// Row getter (read pin and get 0 at the pressed row)
int get_row(void)
{
	// Initialize pins to allow for columns as input
	init_rows();
	
	//Active low to get the row
	if(!HAL_GPIO_ReadPin(ROW_GPIO, ROW1))
	{
		return 0;
	}
	else if(!HAL_GPIO_ReadPin(ROW_GPIO, ROW2))
	{
		return 1;
	}
	else if(!HAL_GPIO_ReadPin(ROW_GPIO, ROW3))
	{
		return 2;
	}
	else if(!HAL_GPIO_ReadPin(ROW_GPIO, ROW4))
	{
		return 3;
	}
	
	else return NON_POLLABLE;
}

// Based on the row and column value, get the number from the map
int get_num_pad_value(void)
{
	int debouce_counter = 0;
	int pressed_key;
	int row;
	int col;
	int i;
	
	int  keypad_map[ROWS][COLS] =
	{
		{1, 2, 3},
		{4, 5, 6},
		{7, 8, 9},
		{10, 0, 11}
	};
	
	row = get_row();
	col = get_col();
	
	if (row != NON_POLLABLE && col != NON_POLLABLE)
	{
		pressed_key = keypad_map[row][col];
	}
	else
	{
		pressed_key = NON_POLLABLE;
	}
	
	//TO HANDLE THE DEBOUNCE:
	//Check the key status first,then will provide a delay of 20 ms (normaliy we keep pressing the key for more than 20 ms) 
	//and again read the key status.if we are getting the same value,we will take it as the key is pressed, else, it is a new value
	return pressed_key;		
}

//--------------------------------------------------------------------------------------MAIN THREAD WORKER FUNCTION
//Worker function that belongs to Keyboard.
void Thread_Keypad (void const *argument) 
{
	keypad_config kc;
	init_keypad_config(&kc);
	int keypad_pressed_num;
	
	//While loop
	while(1)
	{		
		// wait for 100hz interrupt coming from TIM3 --> BLOCKING HERE UNTIL SIGNAL FROM TIM RECEIVED
		osSignalWait(0x0007, osWaitForever);
		
		keypad_pressed_num = get_num_pad_value();
		get_kb_value(keypad_pressed_num, &kc);
		
		if (kc.number != NON_POLLABLE)
		{
			if(kc.number == TEMPERATURE_MODE)
			{
				// Setting temperature mode for 7 segemnt display
				selected_mode = 1;
			}
			
			if(kc.number == ROLL_MODE)
			{
				// Setting accelerometer mode for 7 segemnt display
				selected_mode = 2;
			}
			
			if(kc.number == PITCH_MODE)
			{
				// Setting accelerometer mode for 7 segemnt display
				selected_mode = 3;
			}
		}
	}
}

//--------------------------------------------------------------------------------------THREAD STARTER
//Temperature Thread started
int start_keypad_thread(void *args) 
{
  tid_Thread_Keypad = osThreadCreate(osThread(Thread_Keypad), args);
	
	if (tid_Thread_Keypad == NULL)
		{                                       
			printf("Failed to create keyboard thread!!!!");
		}	
	else
	{
		printf("Successfully started temperature polling thread\n!");
	}
  return(0);
}

