////////////////////////////////////////////////////////////////////////////////
//	File Name					: main.c
//	Description				: program entry
//	Author						: Harsh Aurora
//	Date							: Oct 28, 2016
////////////////////////////////////////////////////////////////////////////////
	
//		Includes		//
#include <stm32f4xx_hal.h>
#include <supporting_functions.h>
#include <sysclk_config.h>
#include <lis3dsh.h>
#include <arm_math.h>
#include <LED_thread.h>
#include <mouse_thread.h>
#include <cmsis_os.h>
#include <rl_usb.h>                     // Keil.MDK-Pro::USB:CORE


//Brief:	main program
//				
//Params:	None
//Return:	None
int main(void) {
  //		MCU Configuration		//
  //	Reset of all peripherals, Initializes the Flash interface and the Systick	//
	osKernelInitialize();  
  HAL_Init();
	
  //	Configure the system clock	//
  SystemClock_Config();
	
	USBD_Initialize(0);               /* USB Device 0 Initialization        */
  USBD_Connect(0); 
	
	start_LED_thread(NULL);
	start_mouse_thread(NULL);
	
	osKernelStart();
	osDelay(osWaitForever);
	return 0;
}
