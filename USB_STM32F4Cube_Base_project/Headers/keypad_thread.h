////////////////////////////////////////////////////////////////////////////////
//	File Name					: keypad_thread.h
//	Description				: header file for num_pad.c
//	Author						: Roman Andoni, Armen Stepanians
//	Date							: Nov 3th, 2016
////////////////////////////////////////////////////////////////////////////////
#ifndef _NUM_PAD
#define _NUM_PAD
#define NON_POLLABLE 69
#define MODE_SELECTION 10
#define ENTER 11

#define TEMPERATURE_MODE 1
#define ROLL_MODE 2
#define PITCH_MODE 3

#define ROWS 4
#define COLS 3
#define DEBOUNCE_DELAY 25 // 50 ms

#define COL_GPIO GPIOC
#define COLUMN1 GPIO_PIN_6 
#define COLUMN2 GPIO_PIN_8 
#define COLUMN3 GPIO_PIN_9


#define ROW_GPIO GPIOC
#define ROW1 GPIO_PIN_11
#define ROW2 GPIO_PIN_2
#define ROW3 GPIO_PIN_4
#define ROW4 GPIO_PIN_5

#define NUM_PAD_PORT_CLK_EN __HAL_RCC_GPIOC_CLK_ENABLE();

#include <cmsis_os.h>

//Thread related to keypad
extern int start_keypad_thread(void *args);

extern osThreadId tid_Thread_Keypad;

//Inits the num pad
void init_num_pad();

//Gets the numpad values
int get_num_pad_value();

//Gets the row of pressed keypad key
int get_row(void);

//Gets the column of pressed keypad key
int get_col(void);

//Initializes columns pins as pull up and row pins as no pull
void init_columns(void);

//Initializes rows pins as pull up and column pins as no pull
void init_rows(void);
#endif