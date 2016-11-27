#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <kalman_filter_funcs.h>

/******************************************************************************************************************************HELPER FUNCTIONS*/
/****************************************************************************** ERROR HANDLER*/
arm_status error_handler(arm_status status)
{
	if ((unsigned char)status != (unsigned char)ARM_MATH_SUCCESS)
		return status;
	else
		return 0;
}
/****************************************************************************** MATRIX FILLER FUNCTION*/
/*Fills matrices in a way (unit matrix * NUMBER)*/
void fill_me_up(int dimension, float value, float* array)
{
	for (int i=0; i< dimension*dimension; i+= dimension + 1)
	{
		*(array + i) = value ;
	}
}
/****************************************************************************** COLUMN EXTRACTOR FUNCTION*/
void extract_col(float* array, float* z, int meas_dimension, int length, int col)
{
	int j = 0;
	for(int i = col; i < meas_dimension * length; i += length){
		*(array + j) = *( z + i);
		j++;
	}
} 

/****************************************************************************** MATRIX UPDATE FUNCTION*/
void update_col(float* x_array, float* x, int state_dimension, int Length, int col)
{
	for(int i = 0; i < state_dimension; i++)
		*(x + i*Length + col) = *(x_array + i);
}

/****************************************************************************** CALCULATES MEAN OF VECTOR*/
float32_t calculate_mean(float* srcA, int Length)
{
	float sum = 0;
	for (int l = 0; l < Length; l++)
		sum += *(srcA + l);
	return sum / Length;
}
 
/****************************************************************************** SUBSTRACT VECTOR A - VECTOR B*/
void substract(float* array, float* dst_array, float value, int Length)
{
	for (int i = 0; i < Length; i++)
		*(dst_array + i) = *(array + i) - value;
}

/****************************************************************************** DOT PRODUCT VECTOR A * VECTOR B*/
void dot_product(float* srcA, float* srcB, int array_length, float* srcC)
{
	float sum = 0;
	float s1 = 0;
	float s2 = 0;
	for (int i = 0; i < array_length; i++)
	{
		s1 = *(srcA + i);
		s2 = *(srcB + i);
		sum += s1 * s2;
	}
	*(srcC) = sum;
}
/******************************************************************************CORRELATION BETWEEN RESIDUALS AND INPUT*/
/**
Step 1: Find the mean of x, and the mean of y
Step 2: Subtract the mean of x from every x value (call them "a"), do the same for y	(call them "b")
Step 3: Calculate: a × b, a2 and b2 for every value
Step 4: Sum up a × b, sum up a2 and sum up b2
Step 5: Divide the sum of a × b by the square root of [(sum of a2) × (sum of b2)]

**/
void calculate_correlations(float* srcA, float* srcB, int state_dimension, int Length, float* correlations)
{
	float32_t axb[state_dimension];
	float32_t temp1[Length];	//a
	float32_t temp2[Length];	//b
	float32_t temp3[state_dimension]; //a2
	float32_t temp4[state_dimension];	//b2
	for (int s = 0; s < state_dimension; s++)
	{
		// get a and b 
		substract(srcA + s*Length, temp1, calculate_mean(srcA + s*Length, Length), Length);
		substract(srcB + s*Length, temp2, calculate_mean(srcB + s*Length, Length), Length);
		// calculate axb
		dot_product(temp1, temp2, Length, axb + s);
		dot_product(temp1, temp1, Length, temp3 + s); //a2
		dot_product(temp2, temp2, Length, temp4 + s);	//b2
	}
	for (int s = 0; s < state_dimension; s++)
	{
		// correlation
		*(correlations + s) = *(axb + s) / sqrt(((*(temp3 + s))*(*(temp4 + s))));
	}
}

/******************************************************************************RESIDUAL STANDARD DEVIATION*/
/**
1. Work out the Mean (the simple average of the numbers)
2. Then for each number: subtract the Mean and square the result
3. Then work out the mean of those squared differences.
4. Take the square root of that and then done.
**/
void calculate_std_dev(float* srcA, float* std_dev, int state_dimension, int Length)
{
	float32_t temp1[Length];
	float32_t temp2[Length]; //the dot product vector of square differences
	float tosquare;
	float squared;
	float final_avg;
	for (int s = 0; s < state_dimension; s++)
	{	
		substract(srcA + s*Length, temp1, calculate_mean(srcA + s*Length, Length), Length);
		
		for (int k =0; k < Length; k++)
		{
			tosquare = *(temp1 +k);
			squared = tosquare * tosquare;
			*(temp2 + k) = squared;
		}
		final_avg = calculate_mean(temp2, Length); 
		*(std_dev + s) = sqrt(final_avg * Length / (Length - 1));
	}
}

void arm_std_dev(float* srcA, float* srcB, int state_dim, int Length)
{
	for (int s = 0; s < state_dim; s++)
		arm_std_f32(srcA + s*Length, Length, srcB + s);
}
/******************************************************************************RESIDUAL MEAN*/
void calculate_residual_mean(float* srcA, float* mean_vector, int state_dimension, int Length)
{
	for (int s = 0; s < state_dimension; s++)
		*(mean_vector + s) = calculate_mean(srcA + s*Length, Length); 
}

void arm_mean(float* srcA, float* srcB, int state_dim, int Length)
{
	for (int s = 0; s < state_dim; s++)
		arm_mean_f32(srcA + s*Length, Length, srcB + s);
}

/******************************************************************************AUTOCORRELATION*/
void calculate_autocorrelation(float* srcA, float* dest, int Lag, int state_dimension, int Length)
{
	int length = Length - Lag;
	for (int s = 0; s < state_dimension; s++)
		calculate_correlations(srcA + Lag + s*length, srcA, state_dimension, length, dest);
	/*
	float mean;
	float sum_numerator = 0;
	float sum_denominator = 0;
	for (int s = 0; s < state_dimension; s++)
	{
		mean = calculate_mean(srcA + s*Length, Length);
		for (int k = 1; k <= Lag; k++)
		{
			for (int lk = k; lk < Length - k; lk++)
				sum_numerator +=(*(srcA + lk + s*Length) - mean)*(*(srcA + lk + k + s*Length) - mean);
			for (int l = 0; l < Length; l++)
				sum_denominator += (*(srcA + l + s*Length) - mean)*(*(srcA + l + s*Length) - mean);
			// calculate autocorrelation and store
			*(dest + s) = sum_numerator / sum_denominator;
		}
	}*/
}
/****************************************************************************************************MAIN KALMAN FUNCTION OPS*/

int Kalmanfilter_C(float* InputArray, float* OutputArray, kalmanstruct_c* kstate,int Length, int State_dimension, int Measurement_dimension)
{
	
	/****************************************************************************** INITIALIZATIONS*/
	// error handling
	// int (*error_ptr)(arm_status);
	// error_ptr = &error_handler;
	// rest of parameters
	float32_t x_col_f32[State_dimension];
	float32_t z_col_f32[Measurement_dimension];
	float32_t fT_f32[State_dimension*State_dimension];
	float32_t hT_f32[State_dimension*State_dimension];
	float32_t temp1_f32[State_dimension*State_dimension];
	float32_t temp2_f32[State_dimension*State_dimension];
	float32_t temp3_f32[State_dimension]; // 3x1
	float32_t temp4_f32[State_dimension]; // 3x1
	//arrays for correlation cals
	float32_t res_f32[State_dimension*Length];
	float32_t cor[State_dimension];
	float32_t std_dev[State_dimension];
	float32_t res_mean[State_dimension];
	float32_t auto_cor[State_dimension];
		
  uint16_t srcRows = State_dimension;
  uint16_t srcColumns = State_dimension;

	//Initing 
	arm_matrix_instance_f32 z_col;	// 3x1
	arm_matrix_instance_f32 x_col;	// 3x1
	arm_matrix_instance_f32 z;
	arm_matrix_instance_f32 x;
	arm_matrix_instance_f32 k;
	arm_matrix_instance_f32 p;
	arm_matrix_instance_f32 r;
	arm_matrix_instance_f32 q;
	arm_matrix_instance_f32 f;
	arm_matrix_instance_f32 fT;
	arm_matrix_instance_f32 h;
	arm_matrix_instance_f32 hT;
	arm_matrix_instance_f32 I;
	arm_matrix_instance_f32 temp1;
	arm_matrix_instance_f32 temp2;
	arm_matrix_instance_f32 temp3; // 3x1
	arm_matrix_instance_f32 temp4; // 3x1
	
	arm_status error_number = ARM_MATH_SUCCESS;
	
	arm_mat_init_f32(&z, Measurement_dimension, Length, (float32_t *)InputArray);
	arm_mat_init_f32(&x, State_dimension, Length, (float32_t *)OutputArray);
	arm_mat_init_f32(&k, srcRows, srcColumns, (float32_t *)kstate->k_f32);
	arm_mat_init_f32(&p, srcRows, srcColumns, (float32_t *)kstate->p_f32);
	arm_mat_init_f32(&f, srcRows, srcColumns, (float32_t *)kstate->f_f32);
	arm_mat_init_f32(&h, srcRows, srcColumns, (float32_t *)kstate->h_f32);
	arm_mat_init_f32(&q, srcRows, srcColumns, (float32_t *)kstate->q_f32);
	arm_mat_init_f32(&r, srcRows, srcColumns, (float32_t *)kstate->r_f32);
	arm_mat_init_f32(&I, srcRows, srcColumns, (float32_t *)kstate->I_f32);
	arm_mat_init_f32(&fT, srcRows, srcColumns, (float32_t *)fT_f32);
	arm_mat_init_f32(&hT, srcRows, srcColumns, (float32_t *)hT_f32);
	
	arm_mat_init_f32(&temp1, srcRows, srcColumns, (float32_t *)temp1_f32);
	arm_mat_init_f32(&temp2, srcRows, srcColumns, (float32_t *)temp2_f32);
	arm_mat_init_f32(&temp3, srcRows, 1, (float32_t *)temp3_f32);	// 3x1
	arm_mat_init_f32(&temp4, srcRows, 1, (float32_t *)temp4_f32);	// 3x1
			
	arm_mat_init_f32(&z_col, srcRows, 1, z_col_f32);
	arm_mat_init_f32(&x_col, srcRows, 1, x_col_f32);
	
	error_number += arm_mat_trans_f32(&f, &fT);
	error_number += arm_mat_trans_f32(&h, &hT);
	
	/******************************************************************************FILTER LOOP*/
	
	for(int c = 0; c < Length; c++){
		extract_col(z_col_f32, InputArray, Measurement_dimension, Length, c);
		extract_col(x_col_f32, OutputArray, Measurement_dimension, Length, c);
		
		// Kalman functions
		// At each operation, use a temporary matrix to avoid overwriting the output (no feedback)
		// Then copy the output from temp onto the variable
		
		//PREDICT//
		
		//X(k|k-1) = F*X(k-1|ka-1)
		error_number += arm_mat_mult_f32(&f, &x_col, &temp3); 		// F*X  (3x1)
		arm_copy_f32(temp3_f32, x_col_f32, State_dimension);			// copy result to X
		
		//P(k|k-1) = F*P(k-1|ka-1)*FT + Q
		error_number += arm_mat_mult_f32(&f, &p, &temp1); 				// F*P
		error_number += arm_mat_mult_f32(&temp1, &fT, &temp2);		// F*P*FT
		error_number += arm_mat_add_f32(&temp2, &q, &p);			 		// P = F*P*FT + Q
		
		
		//UPDATE//
		
		// K new = P(k|k-1)*hT*(h*P(k|k-1)*hT + R)^-1
		error_number += arm_mat_mult_f32(&p, &hT, &temp1); 					// P*hT
		error_number += arm_mat_mult_f32(&h, &p, &k);								// h*P
		error_number += arm_mat_mult_f32(&k, &hT, &temp2);					// h*P*hT
		error_number += arm_mat_add_f32(&temp2, &r, &k); 						// h*P*hT + R
		error_number += arm_mat_inverse_f32(&k, &temp2);						// (h*P*hT + R)^-1
		error_number += arm_mat_mult_f32(&temp1, &temp2, &k);				// P(k|k-1)*hT*(h*P(k|k-1)*hT + R)^-1
		
		// P(new) = (I - K*h)*P
		error_number += arm_mat_mult_f32(&k, &h, &temp1);						// KH
		error_number += arm_mat_sub_f32(&I, &temp1, &temp2);				// I - KH
		error_number += arm_mat_mult_f32(&temp2, &p, &temp1);				// (I - K*h)*P
		arm_copy_f32(temp1_f32, kstate->p_f32, (State_dimension*State_dimension));
		
		// X = X(k-1) + K(z - HX(k-1))
		error_number += arm_mat_mult_f32(&h, &x_col, &temp3);				// HX (3x1)
		error_number += arm_mat_sub_f32(&z_col, &temp3, &temp4);		// z - HX (3x1)
		update_col(temp4_f32, res_f32, State_dimension, Length, c);	// store residual in corresponding res coloumn
		error_number += arm_mat_mult_f32(&k, &temp4, &temp3);				// K(z - HX) (3x1)
		error_number += arm_mat_add_f32(&x_col, &temp3, &x_col);		// X = X(k-1) + K(z - HX(k-1)) (3x1)
		
		update_col(x_col_f32, OutputArray, State_dimension, Length, c);
	}
	
	if (error_number < 0)
		return -1;
	
	return ARM_MATH_SUCCESS;
}

void initKalman(float q, float r, kalmanstruct_c* kc)
{
	fill_me_up(STATE_DIM, 0.1 , kc->p_f32);
	fill_me_up(STATE_DIM, 0.0, kc->k_f32);
	fill_me_up(STATE_DIM, q, kc->q_f32);
	fill_me_up(STATE_DIM, r, kc->r_f32);
	fill_me_up(STATE_DIM, 1   , kc->f_f32);
	fill_me_up(STATE_DIM, 1.0 , kc->h_f32);
	fill_me_up(STATE_DIM, 1.0 , kc->I_f32);
}

void runKalman(float* INPUT_ARRAY, float* OUTPUT_ARRAY, kalmanstruct_c* kc)
{
	arm_status status = Kalmanfilter_C((float32_t *)INPUT_ARRAY, OUTPUT_ARRAY, kc, MEAS_LEN, STATE_DIM, MEAS_DIM);
	// Setting back the original temperature to the 'Kalmanized' temperature
	
}
