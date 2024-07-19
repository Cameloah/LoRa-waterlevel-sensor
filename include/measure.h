#include "Arduino.h"
#include <Wire.h>
#include <SparkFun_VL53L5CX_Library_Constants.h>
#include <SparkFun_VL53L5CX_Library.h>
#include <cmath>


#define NUM_MEASUREMENTS                                    1
#define PATTERN_SIZE                                        15

int get_fill_status();

void _convolution(int input[10][10], int kernel[3][3], int input_rows, int input_cols, int kernel_rows, int kernel_cols);
void _patternMatching(int input[8][8], int input_rows, int input_cols, int pattern_rows, int pattern_cols, int* intersection, int* best_angle);
void _rotate(int input[PATTERN_SIZE][PATTERN_SIZE], int output[PATTERN_SIZE][PATTERN_SIZE], double angle);