////////////////////////////////////////////////////////////////////////////////
//	File Name					: accelerometer.c
//	Description				: Accelerometer with thread
//	Author						: Roman Andoni, Armen Stepanians
//	Date							: Nov 3th, 2016
////////////////////////////////////////////////////////////////////////////////

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "supporting_functions.h"
#include "lis3dsh.h"
#include "main.h"
#include "accelerometer_thread.h"
#include "TIM.h"
#include "segment_display.h"
#include "keypad_thread.h"
#include <stdio.h>
#include <stdlib.h>

//--------------------------------------------------------------------------------------Defines 
// For calculating pitch/roll
#include "math.h"
#define PI		3.14159265358979323846 
#define X_OFFSET -31.876160
#define Y_OFFSET -10.339531
#define Z_OFFSET 21.136719
//--------------------------------------------------------------------------------------Globals 
// Private struct init type for accel
LIS3DSH_InitTypeDef AccelInit;
LIS3DSH_DRYInterruptConfigTypeDef AccelInterrupt;
GPIO_InitTypeDef AccelGPIO; 

void Thread_Accelerometer (void const *argument);
osThreadId tid_Thread_Accelerometer;                        // thread id
osThreadDef(Thread_Accelerometer, osPriorityNormal, 1, 0);	// thread name, priority, instances, stack size

kalmanstruct_c kalman1;
kalmanstruct_c kalman2; 
kalmanstruct_c kalman3;

//--------------------------------------------------------------------------------------Config 
//Configuration for accelerometer and EXTI init
void AccelConfig(void){
	// Enable the clock for Port E
	__HAL_RCC_GPIOE_CLK_ENABLE();
	
	// Init LIS3DSH (accel) struct
	AccelInit.Power_Mode_Output_DataRate = LIS3DSH_DATARATE_25; 	
	AccelInit.Axes_Enable = LIS3DSH_XYZ_ENABLE; 									
	AccelInit.Continous_Update = LIS3DSH_ContinousUpdate_Disabled;
	AccelInit.AA_Filter_BW = LIS3DSH_AA_BW_50; 									
	AccelInit.Full_Scale = LIS3DSH_FULLSCALE_2; 									
	
	// Use accel init funtion
	LIS3DSH_Init(&AccelInit);
	
	// Init LIS3DSH (accel interrupt) struct
	AccelInterrupt.Dataready_Interrupt = LIS3DSH_DATA_READY_INTERRUPT_ENABLED;	// Enable interrupt 
	AccelInterrupt.Interrupt_signal = LIS3DSH_ACTIVE_HIGH_INTERRUPT_SIGNAL;			// Active high interrupt
	AccelInterrupt.Interrupt_type = LIS3DSH_INTERRUPT_REQUEST_PULSED;						// Pulsed (non-latch) type
	
	// User accel interrupt function
	LIS3DSH_DataReadyInterruptConfig(&AccelInterrupt);
	
	// GPIO struct for interrupt signal - EXTI_Line0 (PE0)
	AccelGPIO.Pin = GPIO_PIN_0;					
	AccelGPIO.Mode = GPIO_MODE_IT_FALLING; 
	AccelGPIO.Pull = GPIO_NOPULL;						
	AccelGPIO.Speed = GPIO_SPEED_MEDIUM; 		// Medium speed for 25Hz
	
	// Init the GPIO
	HAL_GPIO_Init(GPIOE, &AccelGPIO);
	
	// Enable interrupts on Line0	
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
	HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 1);
}


//Pitch value getter
float calcPitch(float* xyz)
{
	float pitch;
	// x = 0 , y = 1, z = 2
	pitch = atan( xyz[0] / sqrt(pow(xyz[1], 2) + pow(xyz[2], 2)) ) * 180/PI;
	
	return pitch;
}

//Roll value getter
float calcRoll(float* xyz)
{
	float roll;
	// x = 0 , y = 1, z = 2
	roll = atan( xyz[1] / sqrt(pow(xyz[0], 2) + pow(xyz[2], 2)) ) * 180/PI;
	
	return roll;
}

//--------------------------------------------------------------------------------------THREAD 
void Thread_Accelerometer (void const *argument) 
{
	AccelConfig();
	
	float AccelOutput[3];
	float filtered[3];
	int DP_ARRAY[3];
	double pitch;
	double roll;
	
	initKalman(0.001,0.1,&kalman1);
	initKalman(0.001,0.1,&kalman2);
	initKalman(0.001,0.1,&kalman3);
	float MEASURED_X[1];
	float MEASURED_Y[1];
	float MEASURED_Z[1];
	while(1)
	{
		osSignalWait(0x0007, osWaitForever);

		// Read acceleration, Calibrate the output, Set the flag 
		LIS3DSH_ReadACC(AccelOutput);
		float *result = AccelOutput;
		float X_accel_pre = result[0] + X_OFFSET;
		float Y_accel_pre = result[1] + Y_OFFSET;
		float Z_accel_pre = result[2] + Z_OFFSET;
		MEASURED_X[0] = X_accel_pre;
		MEASURED_Y[0] = Y_accel_pre;
		MEASURED_Z[0] = Z_accel_pre;
		runKalman(MEASURED_X, &filtered[0], &kalman1);
		runKalman(MEASURED_Y, &filtered[1], &kalman2);
		runKalman(MEASURED_Z, &filtered[2], &kalman3);
		
		// Converting 
		double tempSum1 = sqrt(filtered[1] * filtered[1] + filtered[2] * filtered[2]);
		double tempSum2 = sqrt(filtered[0] * filtered[0] + filtered[2] * filtered[2]);
		pitch = (180/PI) * atan((double) filtered[1] / tempSum2);
		roll  = (180/PI) * atan((double) filtered[0] / tempSum1);
		
		//Normalize
		roll  += 90;
		pitch += 90;
		
		//MUTEX ACCESS TO SET ANGLE VALUE
		if(selected_mode == ROLL_MODE)
			SevenSegment_SetDisplayValue_Angle(roll);
		else if(selected_mode == PITCH_MODE)
			SevenSegment_SetDisplayValue_Angle(pitch);
	}
}
//--------------------------------------------------------------------------------------THREAD STARTER
//Temperature Thread started
int start_accelerometer_thread(void *args) 
{
  tid_Thread_Accelerometer = osThreadCreate(osThread(Thread_Accelerometer), args);
	
	if (tid_Thread_Accelerometer == NULL)
		{                                       
			printf("Failed to create accelerometer thread!!!!");
		}	
	else
	{
		printf("Successfully started temperature polling thread\n!");
	}
  return(0);
}

// Callback from EXTI external interrupts coming from the accelerometer
// Sending to 7 segment display
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == GPIO_PIN_0)
	{
		osSignalSet(tid_Thread_Accelerometer, 0x0007);
	}
}