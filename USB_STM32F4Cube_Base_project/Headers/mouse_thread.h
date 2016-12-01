////////////////////////////////////////////////////////////////////////////////
//	File Name					: mouse_thread.h
//	Description				: Header file for mouse thread
//	Author						: Harsh Aurora
//	Date							: Nov 8, 2016
////////////////////////////////////////////////////////////////////////////////

#ifndef _MOUSE_THREAD
#define _MOUSE_THREAD

#include <stdint.h>
#include <transceiver_thread.h>
#include <cmsis_os.h>

extern uint8_t  mouse_in_report[4];
extern osThreadId mouse_thread_ID;

//		Exported Functios		//
void start_mouse_thread(void *args);

// Acceleromter calls this to set the roll and pitch value.
void set_roll_and_pitch_to_mouse(int acc_roll,int acc_pitch);

// Getting roll value previously set by accelerometer
int get_roll_from_accelerometer();

// Getting pitch value previously set by accelerometer
int get_pitch_from_accelerometer();

#endif
