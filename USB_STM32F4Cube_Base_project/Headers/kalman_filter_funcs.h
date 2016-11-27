////////////////////////////////////////////////////////////////////////////////
//	File Name					: kalman_filter_funcs.h
//	Description				: Kalman filter functions header file from lab 1 and 2
//	Author						: Roman Andoni, Armen Stepanians
//	Date							: Oct 1st, 2016
////////////////////////////////////////////////////////////////////////////////

#ifndef KALMAN_FILTER_FUNCS_H
#define KALMAN_FILTER_FUNCS_H

#include <arm_math.h>

#define STATE_DIM 1
#define MEAS_DIM 1
#define MEAS_LEN 1

// STRUCT FOR C (Kalman part)
typedef struct kalmanstruct_c
{
	float32_t p_f32[STATE_DIM * STATE_DIM];
	float32_t k_f32[STATE_DIM * STATE_DIM];
	float32_t q_f32[STATE_DIM * STATE_DIM];
	float32_t r_f32[STATE_DIM * STATE_DIM];
	float32_t f_f32[STATE_DIM * STATE_DIM];
	float32_t h_f32[STATE_DIM * STATE_DIM];
	float32_t I_f32[STATE_DIM * STATE_DIM];
}kalmanstruct_c;

/****************HELPER AND CALCULATION FUNCTIONS*************************/
void substract(float* array, float* dst_array, float value, int Length);

void dot_product(float* srcA, float* srcB, int array_length, float* srcC);

void calculate_correlations(float* srcA, float* srcB, int state_dimension, int Length, float* correlations);

void calculate_std_dev(float* srcA, float* std_dev, int state_dimension, int Length);

void arm_std_dev(float* srcA, float* srcB, int state_dim, int Length);

void calculate_residual_mean(float* srcA, float* mean_vector, int state_dimension, int Length);

void arm_mean(float* srcA, float* srcB, int state_dim, int Length);

void calculate_autocorrelation(float* srcA, float* dest, int Lag, int state_dimension, int Length);

/****************ERROR HANDLER*******************************************/
arm_status error_handler(arm_status status);

/****************KALMAN RELATED FUNCTIONS********************************/
void fill_me_up(int dimension, float value, float* array);

void extract_col(float* array, float* z, int meas_dimension, int length, int col);

void update_col(float* x_array, float* x, int state_dimension, int Length, int col);

float32_t calculate_mean(float* srcA, int Length);	

int Kalmanfilter_C(float* InputArray, float* OutputArray, struct kalmanstruct_c *kstate,int Length, int State_dimension, int Measurement_dimension);

void runKalman(float* INPUT_VALUE, float* OUTPUT_VALUE, kalmanstruct_c* kc);

void initKalman(float q, float r, kalmanstruct_c* kc);

#endif
