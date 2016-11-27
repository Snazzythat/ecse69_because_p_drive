////////////////////////////////////////////////////////////////////////////////
//	File Name					: accelerometer.h
//	Description				: Accelerometer with thread
//	Author						: Roman Andoni, Armen Stepanians
//	Date							: Nov 3th, 2016
////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <inttypes.h>
#include <cmsis_os.h>
#include "kalman_filter_funcs.h"

#ifndef _ACCELEROMETER
#define _ACCELEROMETER

//Thread related to keypad
extern int start_accelerometer_thread(void *args);

//Thread ID extern
extern osThreadId tid_Thread_Accelerometer;

// For Accelerometer config
void AccelConfig (void);
// Pitch calculator
float calcPitch(float* xyz);
// Roll calculator
float calcRoll(float* xyz);
#endif