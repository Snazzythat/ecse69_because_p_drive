////////////////////////////////////////////////////////////////////////////////
//	File Name					: mouse.c
//	Description				: Example of an OS thread controlling the mouse USB 
//											interface via the on-board pushbutton
//	Author						: Harsh Aurora
//	Date							: Nov 8, 2016
////////////////////////////////////////////////////////////////////////////////
	
//		Includes		//
#include <mouse_thread.h> 
#include <stm32f4xx_hal.h>
#include <cmsis_os.h>
#include <rl_usb.h>

//		Function Declaration		//
void mouse_thread(void const *args);

//		Globals 		//
osThreadId mouse_thread_ID;
osThreadDef(mouse_thread, osPriorityNormal, 1,0);
uint8_t  mouse_in_report[4] = {0,0,0,0};

int roll;
int pitch;

//Mutex for mouse to set and get roll and pitch values
osMutexId mouse_mutex;
osMutexDef(mouse_mutex);

//Brief:		Initializes the GPIO for the pushbutton
//Params:		None
//Return:		None
void mouse_thread_periph_init(void) {
	GPIO_InitTypeDef mouse_GPIO_struct;
	
	__HAL_RCC_GPIOC_CLK_ENABLE();

	mouse_GPIO_struct.Pin		= GPIO_PIN_6;
	mouse_GPIO_struct.Mode	= GPIO_MODE_INPUT;
	mouse_GPIO_struct.Pull	= GPIO_NOPULL;
	mouse_GPIO_struct.Speed	= GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(GPIOC, &mouse_GPIO_struct);
	
	mouse_GPIO_struct.Pin		= GPIO_PIN_8;
	mouse_GPIO_struct.Mode	= GPIO_MODE_INPUT;
	mouse_GPIO_struct.Pull	= GPIO_NOPULL;
	mouse_GPIO_struct.Speed	= GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(GPIOC, &mouse_GPIO_struct);
	
	mouse_GPIO_struct.Pin		= GPIO_PIN_9;
	mouse_GPIO_struct.Mode	= GPIO_MODE_INPUT;
	mouse_GPIO_struct.Pull	= GPIO_NOPULL;
	mouse_GPIO_struct.Speed	= GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(GPIOC, &mouse_GPIO_struct);
}

//Brief:		Starts the mouse thread in the OS (from Inactive into the Lifecycle)
//Params:		A void pointer to initial arguments, NULL if unused
//Return:		None
void start_mouse_thread(void *args) {
	mouse_thread_ID = osThreadCreate(osThread(mouse_thread), args);
}

// Acceleromter calls this to set the roll and pitch value.
void set_roll_and_pitch_to_mouse(int acc_roll, int acc_pitch)
{
	/* We have race conditions. Need to use mutexes to protect these global flags. */
	osMutexWait(mouse_mutex, osWaitForever);
	roll = acc_roll;
	pitch = acc_pitch;
	osMutexRelease(mouse_mutex);
}

// Getting roll value previously set by accelerometer
int get_roll_from_accelerometer()
{
	int return_roll;
	
	/* We have race conditions. Need to use mutexes to protect these global flags. */
	osMutexWait(mouse_mutex, osWaitForever);
	return_roll = roll;
	osMutexRelease(mouse_mutex);
	
	return return_roll;
}

// Getting pitch value previously set by accelerometer
int get_pitch_from_accelerometer()
{
	int return_pitch;
	
	/* We have race conditions. Need to use mutexes to protect these global flags. */
	osMutexWait(mouse_mutex, osWaitForever);
	return_pitch = pitch;
	osMutexRelease(mouse_mutex);
	
	return return_pitch;
}


//Brief:		The mouse thread function in the OS
//					Performs a mouse click if the userbutton is pressed
//Params:		A void pointer to initial arguments, NULL if unused
//Return:		None
void mouse_thread(void const *args) {
	
	int mouse_X;
	int mouse_Y;
	
	mouse_in_report[0] = 0;
	mouse_in_report[1] = 0;
	mouse_in_report[2] = 0;
	mouse_in_report[3] = 0;
	
	mouse_thread_periph_init();
	
	while(1) {
		// wait for thread signal from TIM
		osSignalWait(0x0069, osWaitForever);
		
		// get mouse relative X and Y
	  mouse_X =  get_roll_from_accelerometer();
		mouse_Y = get_pitch_from_accelerometer();
		
		// assign X to report
		if (mouse_X < 0)
			mouse_in_report[1] = (uint8_t)mouse_X;
		else
			mouse_in_report[1] = mouse_X;
		// assign Y to report
		if (mouse_Y < 0)
			mouse_in_report[2] = (uint8_t)mouse_Y;
		else
			mouse_in_report[2] = mouse_Y;
		
		// button pressed (LMB, RMB, SCROLL)
		if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6) == SET)
			mouse_in_report[0] = 0x01;
		else if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_8) == SET)
			mouse_in_report[0] = 0x02;
		else if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9) == SET)
			mouse_in_report[0] = 0x04;
		else
			mouse_in_report[0] = 0x00;
		
		// send in the report
		USBD_HID_GetReportTrigger(0, 0, mouse_in_report, 4);		
	}
}

