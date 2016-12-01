////////////////////////////////////////////////////////////////////////////////
//	File Name				: transceiver_thread.h
//	Description			: program header
//	Author					: Team 12
//	Date						: Nov 29th, 2016
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <cmsis_os.h>


#ifndef _TRANSCEIVER_THREAD_H
#define _TRANSCEIVER_THREAD_H

extern osThreadId transceiver_thread_ID;

extern uint8_t mouse_report_array_set[4];
extern uint8_t mouse_report_array_get[4];

//	Exported Functions
extern void start_transceiver_thread(void *args);

void transceiver_thread(void const *args);

// mouse thread calls this to set the report array
void set_report_array(uint8_t*);

// Getting report array previously set by mouse thread
void get_report_array();
#endif	/*_TRANSCEIVER_THREAD_H*/
