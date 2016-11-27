
#include <cmsis_os.h>

#ifndef _SEGMENT_DISPLAY
#define _SEGMENT_DISPLAY
#define SEVEN_SEGMENT_DISPLAY_PORT GPIOB
#define DISPLAY_0 GPIO_PIN_15 // Pin B15 is connected to the 7 segment display 1
#define DISPLAY_1 GPIO_PIN_8 // Pin B8 is connected to the 7 segment display 2
#define DISPLAY_2 GPIO_PIN_4 // Pin B4 is connected to the 7 segment display 3
#define DISPLAY_3 GPIO_PIN_6 // Pin B6 is connected to the 7 segment display 4
#define SEVEN_SEG_DISP_CLK_EN __HAL_RCC_GPIOB_CLK_ENABLE(); // Clock for display and display select (both will use B pins cluster)

#define L 	-1
#define H 	-2
#define E		-3
#define c		-4
#define o		-5
#define OFF -69

//		Function Declaration		//
void segment_thread(void const *args);

//		Globals 		//
extern osThreadId segment_thread_ID;

// Initializes seven segment selection selection. 
void seven_segment_select_init(int);

// Calls the selection of the display 
void seven_segment_select(int);

// Initializes seven segment display itself. 
void seven_segment_init();

// Displays the value on the 7 segemnt display
void run_segment_display(int);

// lol
void start_segment_thread();

//Getter for temperature sensor decimals for 7 segment display
void get_processor_temp(int *dec1, int *dec2, int *dec3, float temp_value);

//Getter for accelerometer angle decimals for 7 segment display
void get_angle_decs(int *dec1, int *dec2, int *dec3, float angle);

//Modes for detection
extern int selected_mode;

//Temperature that will be displayed on the 7 segemnt display
extern float filteredTemp;

// Angle (pitch or roll) that will be set externally by Accelerometer
extern float displayed_angle;

// Mutex setter for temperature
void SevenSegment_SetDisplayValue_Temp(float temp);

// Mutex getter for temperature
float SevenSegment_GetDisplayValue_Temp(void);

// Mutex setter for angle
void SevenSegment_SetDisplayValue_Angle(float angle);

// Mutex getter for angle
float SevenSegment_GetDisplayValue_Angle(void);
	
#endif
