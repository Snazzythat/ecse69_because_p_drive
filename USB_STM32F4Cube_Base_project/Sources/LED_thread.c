////////////////////////////////////////////////////////////////////////////////
//	File Name					: LED_thread.c
//	Description				: Example of an OS thread that toggles the board LED's
//											based on a 1 second interrupt from TIM3
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
TIM_HandleTypeDef TIM3_handle;

//Brief:		Initializes the GPIO and TIM periphs used in this example
//					GPGIOs : D12, D13, D14, D15 as output (LED GPIOs)
//					TIM:	TIM3 to provide 1 second interrupts
//Params:		None
//Return:		None
void LED_thread_periph_init(void) {
	GPIO_InitTypeDef LED_GPIO_struct;
	
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_TIM3_CLK_ENABLE();
	
	LED_GPIO_struct.Pin		= GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
	LED_GPIO_struct.Mode	= GPIO_MODE_OUTPUT_PP;
	LED_GPIO_struct.Pull	= GPIO_PULLDOWN;
	LED_GPIO_struct.Speed	= GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(GPIOD, &LED_GPIO_struct);
	
	
	TIM3_handle.Instance = TIM3;
	TIM3_handle.Init.Prescaler					= 20999;        
	TIM3_handle.Init.CounterMode				= TIM_COUNTERMODE_DOWN;     
	TIM3_handle.Init.Period							= 3999;           
	TIM3_handle.Init.ClockDivision			= TIM_CLOCKDIVISION_DIV1;    
	TIM3_handle.Init.RepetitionCounter	= 0;
	
	HAL_TIM_Base_Init(&TIM3_handle);
	HAL_TIM_Base_Start_IT(&TIM3_handle);
	
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
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

//Brief:		The TIM interrupt callback. Sends a signal to the LED_thread
//					if the interrupt was recived from TIM3
//Params:		Pointer to the TIM handle that caused the interrupt
//Return:		None
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {	
	if(htim->Instance == TIM3)
		osSignalSet(LED_thread_ID, 0x00000001);
}
