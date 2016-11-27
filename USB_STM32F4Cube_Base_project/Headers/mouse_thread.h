////////////////////////////////////////////////////////////////////////////////
//	File Name					: mouse_thread.h
//	Description				: Header file for mouse thread
//	Author						: Harsh Aurora
//	Date							: Nov 8, 2016
////////////////////////////////////////////////////////////////////////////////

#ifndef _MOUSE_THREAD
#define _MOUSE_THREAD

#include <stdint.h>
extern uint8_t  mouse_in_report[4];

//		Exported Functios		//
void start_mouse_thread(void *args);

#endif
