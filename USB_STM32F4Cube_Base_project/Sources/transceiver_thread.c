////////////////////////////////////////////////////////////////////////////////
//	File Name				: transceiver_thread.c
//	Description			: program entry 
//	Author					: Team 12
//	Date						: Nov 29th, 2016
////////////////////////////////////////////////////////////////////////////////

// Includes
#include "transceiver_thread.h"
#include "transceiver_protocol.h"

osThreadId transceiver_thread_ID;

//Mutex to set and get report arrays
osMutexId transceiver_mutex;
osMutexDef(transceiver_mutex);

uint8_t mouse_report_array_set[4];
uint8_t mouse_report_array_get[4];

osThreadDef(transceiver_thread, osPriorityNormal, 1,0);

// mouse thread calls this to set the report array
void set_report_array(uint8_t *report_array)
{
	/* We have race conditions. Need to use mutexes to protect these global flags. */
	osMutexWait(transceiver_mutex, osWaitForever);
	for (int i = 0; i < 4; i++)
		mouse_report_array_set[i] = report_array[i];
	osMutexRelease(transceiver_mutex);
}

// Getting report array previously set by mouse thread
void get_report_array()
{
	/* We have race conditions. Need to use mutexes to protect these global flags. */
	osMutexWait(transceiver_mutex, osWaitForever);
	for (int i = 0; i < 4; i++)
		mouse_report_array_get[i] = mouse_report_array_set[i];
	osMutexRelease(transceiver_mutex);
}

/*
** Brief:			Starts the display thread in the OS (from Inactive into the Lifecycle)
** Params:		A void pointer to initial arguments, NULL if unused
** Return:		None
*/
void start_transceiver_thread(void *args)
{
	transceiver_thread_ID = osThreadCreate(osThread(transceiver_thread), args);
}

/*
** Brief:			The display thread function in the OS
**						Waits for a signal from the TIM4 interrupt handler and then 
**						toggles the on board LEDs
** Params:		A void pointer to initial arguments, NULL if unused
** Return:		None
*/
void transceiver_thread(void const *args)
{
//	int x = 10;
//	uint8_t temp[6] = {0x09, 0x01, 0xfa, 0x02, 0x02, 0x90};
//	uint8_t temp2 = 0x00;
//	//uint8_t temp3 = 0x10;
	
	while(1)
	{
		uint8_t state;
		osSignalWait(0x07, osWaitForever);
		get_report_array();
		burst_transmit(mouse_report_array_get, 4);
		/////////////////////////////////////////////////RECEIVING
		
//	CC2500_read_one(CC2500_SRX);
//	
//	temp2 = CC2500_get_state();
//	
//	printf("SFM state:					%d\n", temp2);
//	temp2 = CC2500_get_rxbytes();
//			
//	printf("RXBytes size:				%d\n", temp2);
//	
////		temp2 = CC2500_read_rx_one();
////		
////		printf("RX Bytes Read:			%d\n", temp2);
//	
//	CC2500_Read(temp, 0x3f|0xC0, x);
//	for (int i = 0; i<x; i++){
//	printf("Packet %d : %x\n", i, temp[i]);
//	}
//		
		/////////////////////////////////////////////////SENDING

//		CC2500_flush_tx();
//		CC2500_read_one(CC2500_SIDLE);
//		temp2 = CC2500_get_state();
//	
//		if (temp2 == 1){
//		CC2500_write_tx(temp, 6);
//		CC2500_read_one(CC2500_STX);
//		}else{
//		/	printf("NOT IN IDDLE");
//		}
	}
}
