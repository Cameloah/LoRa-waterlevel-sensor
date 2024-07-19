#include "Arduino.h"
#include <Wire.h>
#include <SparkFun_VL53L5CX_Library_Constants.h>
#include <SparkFun_VL53L5CX_Library.h>
#include <cmath>


#define NUM_MEASUREMENTS                                    1
#define PATTERN_SIZE                                        15

int get_fill_status();

/**
 * Performs convolution operation on the input matrix using the given kernel.
 * 
 * @param input The input matrix.
 * @param kernel The convolution kernel.
 * @param input_rows The number of rows in the input matrix.
 * @param input_cols The number of columns in the input matrix.
 * @param kernel_rows The number of rows in the kernel.
 * @param kernel_cols The number of columns in the kernel.
 */
void _convolution(int input[10][10], int kernel[3][3], int input_rows, int input_cols, int kernel_rows, int kernel_cols);

/**
 * Performs pattern matching on the input matrix using the given pattern.
 * 
 * @param input The input matrix.
 * @param input_rows The number of rows in the input matrix.
 * @param input_cols The number of columns in the input matrix.
 * @param pattern_rows The number of rows in the pattern.
 * @param pattern_cols The number of columns in the pattern.
 * @param intersection Pointer to store the intersection value.
 * @param best_angle Pointer to store the best angle.
 */
void _patternMatching(int input[8][8], int input_rows, int input_cols, int pattern_rows, int pattern_cols, int* intersection, int* best_angle);

/**
 * Rotates the input matrix by the given angle.
 * 
 * @param input The input matrix.
 * @param output The output matrix to store the rotated matrix.
 * @param angle The angle of rotation in degrees.
 */
void _rotate(int input[PATTERN_SIZE][PATTERN_SIZE], int output[PATTERN_SIZE][PATTERN_SIZE], double angle);