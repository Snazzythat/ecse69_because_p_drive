////////////////////////////////////////////////////////////////////////////////
//	File Name					: LED_thread.c
//	Description				: Example of an OS thread that toggles the board LED's
//											based on a 1 second interrupt from TIM4
//	Author						: Harsh Aurora
//	Date							: Oct 28, 2016
////////////////////////////////////////////////////////////////////////////////
	
//		Includes		//
#include <LED_thread.h> 
#include <stm32f4xx_hal.h>
#include <cmsis_os.h>

//		Function Declaration		//
void LED_thread(void const *args);

//		Globals 		//
osThreadId LED_thread_ID;
osThreadDef(LED_thread, osPriorityNormal, 1,0);

//Brief:		Initializes the GPIO periph used in this example
//					GPGIOs : D12, D13, D14, D15 as output (LED GPIOs)
//Params:		None
//Return:		None
void LED_thread_periph_init(void) {
	GPIO_InitTypeDef LED_GPIO_struct;
	
	__HAL_RCC_GPIOD_CLK_ENABLE();
	
	LED_GPIO_struct.Pin		= GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
	LED_GPIO_struct.Mode	= GPIO_MODE_OUTPUT_PP;
	LED_GPIO_struct.Pull	= GPIO_PULLDOWN;
	LED_GPIO_struct.Speed	= GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(GPIOD, &LED_GPIO_struct);
}

//Brief:		Starts the LED thread in the OS (from Inactive into the Lifecycle)
//Params:		A void pointer to initial arguments, NULL if unused
//Return:		None
void start_LED_thread(void *args) {
	LED_thread_ID = osThreadCreate(osThread(LED_thread), args);
}

//Brief:		The LED thread function in the OS
//					Waits for a signal from the TIM3 interrupt handler and then 
//					toggles the on board LEDs
//Params:		A void pointer to initial arguments, NULL if unused
//Return:		None
void LED_thread(void const *args) {
	LED_thread_periph_init();
	while(1) {
		osSignalWait(0x00000001, osWaitForever);
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);
		printf("Toggle LEDs\n");
	}
}
