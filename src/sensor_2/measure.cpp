#include "measure.h"


SparkFun_VL53L5CX myImager;
VL53L5CX_ResultsData measurementData; // Result data class structure, 1356 byes of RAM

int kernel_edge[3][3] = {
    {0, -1, 0},
    {-1, 4, -1},
    {0, -1, 0}
};

int pattern[15][15] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
    {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
    {2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2},
    {0, 0, 0, 0, 0, 0, 1, 4, 1, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 1, 4, 1, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 1, 4, 1, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 1, 4, 1, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 1, 4, 1, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 1, 4, 1, 0, 0, 0, 0, 0, 0}
};

int sensor_data[8][8] = {0};
int processed_data[8][8] = {0};


int get_fill_status() {
    Wire.begin();
    Wire.setClock(1000000); //Sensor has max I2C freq of 1MHz
    

    Serial.println("Initializing sensor board...");
    if (!myImager.begin()) {
        Serial.println(F("Sensor not found - check your wiring. Freezing"));
        return -1;
    }
    
    myImager.setResolution(8*8); //Enable all 64 pads
    int imageResolution = myImager.getResolution();
    int imageWidth = sqrt(imageResolution);
    myImager.startRanging();

    int num_measurements = 0;

    while(num_measurements < NUM_MEASUREMENTS) {
        if (myImager.isDataReady() == true)
        {
            if (myImager.getRangingData(&measurementData)) {
                for (int i = 0 ; i < imageResolution; i++)
                {
                    int result = measurementData.distance_mm[i];
                    sensor_data[i / imageWidth][i % imageWidth] = result;
                }

                num_measurements++;
            }
        }

        delay(5);
    }

    myImager.stopRanging();

    if(!myImager.setPowerMode(SF_VL53L5CX_POWER_MODE::SLEEP))
		Serial.print("vl53l5cx_set_power_mode failed\n");

    // Check if device is actually in sleep mode
    SF_VL53L5CX_POWER_MODE currentPowerMode = myImager.getPowerMode();
    switch (currentPowerMode)
    {
        case SF_VL53L5CX_POWER_MODE::SLEEP:
            Serial.println(F("Sensor is sleeping."));
            break;

        case SF_VL53L5CX_POWER_MODE::WAKEUP:
            Serial.println(F("Device is awake."));
            break;

        default:
            Serial.println(F("Cannot retrieve device power mode."));
    }

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            processed_data[i][j] = sensor_data[i][j];
            Serial.print("\t");
            Serial.print(processed_data[i][j]);
        }
        Serial.println();
    }



    int min_value = INT_MAX;
    int max_value = 0;

    // Find minimum and maximum values in processed_data
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (processed_data[i][j] == 0)
                continue;

            if (processed_data[i][j] < min_value) {
                min_value = processed_data[i][j];
            }
            if (processed_data[i][j] > max_value) {
                max_value = processed_data[i][j];
            }
        }
    }



    // Normalize and invert processed_data using min-max normalization formula
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (processed_data[i][j] == 0)
                continue;

            processed_data[i][j] = 255 * (processed_data[i][j] - min_value) / (max_value - min_value);
            if (processed_data[i][j] < 18) // 7% of 255
                processed_data[i][j] = 1;
            else
                processed_data[i][j] = 0;
        }
    }

    Serial.println("\n After normalization and inversion: ");

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Serial.print("\t");
            Serial.print(processed_data[i][j]);
        }
        Serial.println();
    }


    // Pad processed_data with edge values
    int padded_data[10][10] = {0};
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (i == 0 && j == 0) {
                padded_data[i][j] = processed_data[0][0];
            } else if (i == 0 && j == 9) {
                padded_data[i][j] = processed_data[0][7];
            } else if (i == 9 && j == 0) {
                padded_data[i][j] = processed_data[7][0];
            } else if (i == 9 && j == 9) {
                padded_data[i][j] = processed_data[7][7];
            } else if (i == 0) {
                padded_data[i][j] = processed_data[0][j - 1];
            } else if (i == 9) {
                padded_data[i][j] = processed_data[7][j - 1];
            } else if (j == 0) {
                padded_data[i][j] = processed_data[i - 1][0];
            } else if (j == 9) {
                padded_data[i][j] = processed_data[i - 1][7];
            } else {
                padded_data[i][j] = processed_data[i - 1][j - 1];
            }
        }
    }

    Serial.println("\n After padding: ");
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            Serial.print("\t");
            Serial.print(padded_data[i][j]);
        }
        Serial.println();
    }



    _convolution(padded_data, kernel_edge, 10, 10, 3, 3);

    Serial.println("\n After egde detection: ");
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            Serial.print("\t");
            Serial.print(padded_data[i][j]);
        }
        Serial.println();
    }



    // crop and copy the result of convolution back to processed_data and threshold it
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            processed_data[i][j] = padded_data[i + 1][j + 1] < 1 ? 0 : 1;
        }
    }

    Serial.println("\n After cropping and thresholding: ");
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Serial.print("  ");
            Serial.print(processed_data[i][j]);
        }
        Serial.println();
    }



    int intersection[2] = {0, 0};
    int best_angle = 0;
    _patternMatching(processed_data, 8, 8, 15, 15, intersection, &best_angle);

    Serial.print("Intersection: "); Serial.print(intersection[0]); Serial.print(", "); Serial.println(intersection[1]);

    int x_1 = int(cos(radians(best_angle + 90)) * 4);
    int y_1 = int(sin(radians(best_angle + 90)) * 4);

    int x_2 = int(cos(radians(best_angle - 45)) * 4);
    int y_2 = int(sin(radians(best_angle - 45)) * 4);

    int x_3 = int(cos(radians(best_angle - 135)) * 4);
    int y_3 = int(sin(radians(best_angle - 135)) * 4);

    Serial.print("x_1: "); Serial.println(x_1); Serial.print("y_1: "); Serial.println(y_1);
    Serial.print("x_2: "); Serial.println(x_2); Serial.print("y_2: "); Serial.println(y_2);
    Serial.print("x_3: "); Serial.println(x_3); Serial.print("y_3: "); Serial.println(y_3);


    int measure_at[3][2] = {
        {intersection[0] - y_1, intersection[1] + x_1},
        {intersection[0] - y_2, intersection[1] + x_2},
        {intersection[0] - y_3, intersection[1] + x_3}
    };

    // Adjust coordinates if they are outside the size of sensor_data
    for (int i = 0; i < 3; i++) {
        measure_at[i][0] = max(0, min(measure_at[i][0], 7));
        measure_at[i][1] = max(0, min(measure_at[i][1], 7));
    }

    Serial.println("Measure at: ");
    for (int i = 0; i < 3; i++) {
        Serial.print("\t");
        Serial.print(measure_at[i][0]);
        Serial.print(", ");
        Serial.println(measure_at[i][1]);
    }

    int measurements[3] = {0};
    for (int i = 0; i < 3; i++) {
        measurements[i] = sensor_data[measure_at[i][0]][measure_at[i][1]];
    }

    Serial.println("Distance in large compartement: " + String(measurements[0]) + " mm");
    Serial.println("Distance in small right compartement: " + String(measurements[1]) + " mm");
    Serial.println("Distance in small left compartement: " + String(measurements[2]) + " mm");


    int tank_full = 300;
    int tank_empty = 2700;

    int tank_level_cm = 30 + ((measurements[0] / 10 - 30) / 2)
                        + ((measurements[1] / 10 - 30) / 4)
                        + ((measurements[2] / 10 - 30) / 4);

    Serial.println("Tank level: " + String(tank_level_cm) + " cm");

    return tank_level_cm;
}


void _convolution(int input[10][10], int kernel[3][3], int input_rows, int input_cols, int kernel_rows, int kernel_cols) {
    
    int temp[10][10] = {0};
    for (int i = 0; i < input_rows; i++) {
        for (int j = 0; j < input_cols; j++) {
            temp[i][j] = input[i][j];
        }
    }

    for (int i = 0; i < input_rows; i++) {
        for (int j = 0; j < input_cols; j++) {
            int convolute_result = 0;
            for (int m = 0; m < kernel_rows; m++) {
                for (int n = 0; n < kernel_cols; n++) {
                    int convolute_index_k = i - kernel_rows / 2 + m;
                    int convolute_index_l = j - kernel_cols / 2 + n;
                    if (convolute_index_k >= 0 && convolute_index_k < input_rows && convolute_index_l >= 0 && convolute_index_l < input_cols) {
                        convolute_result += temp[convolute_index_k][convolute_index_l] * kernel[m][n];
                    }
                }
            }
            input[i][j] = convolute_result;
        }
    }
}



void _patternMatching(int input[8][8], int input_rows, int input_cols, int pattern_rows, int pattern_cols, int* intersection, int* best_angle) {
    int highest_score = 0;
    *best_angle = -1;
    int coord_intersection[] = {0, 0};

    for (int angle = 0; angle < 360; angle += 5) {
        int kernel[15][15] = {0};
        _rotate(pattern, kernel, angle);
        for (int i = 0; i < input_rows; i++) {
            for (int j = 0; j < input_cols; j++) {
                int convolute_result = 0;
                for (int m = 0; m < pattern_rows; m++) {
                    for (int n = 0; n < pattern_cols; n++) {
                        int convolute_index_k = i - pattern_rows / 2 + m;
                        int convolute_index_l = j - pattern_cols / 2 + n;
                        if (convolute_index_k >= 0 && convolute_index_k < input_rows && convolute_index_l >= 0 && convolute_index_l < input_cols) {
                            convolute_result += input[convolute_index_k][convolute_index_l] * kernel[m][n];
                        }
                    }
                }
                if (convolute_result > highest_score) {
                    highest_score = convolute_result;
                    *best_angle = angle;
                    coord_intersection[0] = i;
                    coord_intersection[1] = j;
                }
            }
        }
    }

    memccpy(intersection, coord_intersection, 2, sizeof(coord_intersection));

    int best_kernel[15][15] = {0};
    _rotate(pattern, best_kernel, *best_angle);
    
    Serial.println("\n Best kernel: ");
        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 15; j++) {
                Serial.print("  ");
                Serial.print(best_kernel[i][j]);
            }
            Serial.println();
        }

    Serial.print("best angle: ");
    Serial.println(*best_angle);
}




void _rotate(int input[PATTERN_SIZE][PATTERN_SIZE], int output[PATTERN_SIZE][PATTERN_SIZE], double angle) {
    double radians = angle * M_PI / 180.0;
    double cosA = cos(radians);
    double sinA = sin(radians);
    
    double centerX = (PATTERN_SIZE - 1) / 2.0;
    double centerY = (PATTERN_SIZE - 1) / 2.0;

    for (int x = 0; x < PATTERN_SIZE; ++x) {
        for (int y = 0; y < PATTERN_SIZE; ++y) {
            // Translate to origin
            double x0 = x - centerX;
            double y0 = y - centerY;

            // Rotate
            double x1 = x0 * cosA - y0 * sinA;
            double y1 = x0 * sinA + y0 * cosA;

            // Translate back
            x1 += centerX;
            y1 += centerY;

            // Interpolation to find the closest pixel value
            int srcX = static_cast<int>(round(x1));
            int srcY = static_cast<int>(round(y1));

            if (srcX >= 0 && srcX < PATTERN_SIZE && srcY >= 0 && srcY < PATTERN_SIZE) {
                output[x][y] = input[srcX][srcY];
            } else {
                // If the transformed coordinates are out of bounds, set the pixel to 0 (or a background value)
                output[x][y] = 0;
            }
        }
    }
}