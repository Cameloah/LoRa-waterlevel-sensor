#include "measure.h"
#include <vector>



#define TANK_LVL_EMPTY_CM                                 270 // in cm

// -------------------- VL53L5CX sensor -------------------- //

SparkFun_VL53L5CX myImager;
VL53L5CX_ResultsData measurementData; // Result data class structure, 1356 byes of RAM



// ------------------- image processing ------------------- //

int kernel_edge[3][3] = {
    {0, -1, 0},
    {-1, 4, -1},
    {0, -1, 0}
};

int kernel_ridge[3][3] = {
    {-1, -1, -1},
    {-1, 8, -1},
    {-1, -1, -1}
};

int kernel_blur[3][3] = {
    {1, 1, 1},
    {1, 8, 1},
    {1, 1, 1}
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
int signal_strength[8][8] = {0};
int sensor_status[8][8] = {0};



// ------------------- functions ------------------- //
int calculate_median(int measurements[], int size) {
        std::vector<int> valid_measurements;
        for (int i = 0; i < size; i++) {
            if (measurements[i] != 0) {
                valid_measurements.push_back(measurements[i]);
            }
        }

        std::sort(valid_measurements.begin(), valid_measurements.end());

        int median = 0;
        if (!valid_measurements.empty()) {
            int mid = valid_measurements.size() / 2;
            if (valid_measurements.size() % 2 == 0) {
                median = (valid_measurements[mid - 1] + valid_measurements[mid]) / 2;
            } else {
                median = valid_measurements[mid];
            }
        }

        else
            return TANK_LVL_EMPTY_CM * 10;

        return median;
    }


int get_fill_status() {

    // setup communication and initialize sensor. Sensor has max I2C freq of 1MHz
    Wire.begin();
    Wire.setClock(1000000);
    
    if (!myImager.begin()) {
        Serial.println(F("Sensor not found - check your wiring. Aborting..."));
        return -1;
    }
    
    myImager.setResolution(8*8);                                    //Enable all 64 pads
    int imageResolution = myImager.getResolution();
    int imageWidth = sqrt(imageResolution);


    // Set the ranging mode
    if (!myImager.setRangingMode(SF_VL53L5CX_RANGING_MODE::AUTONOMOUS))
    {
        Serial.println(F("Cannot set ranging mode requested. Aborting..."));
        return -1;
    }


    // Set the ranging frequency
    if (!myImager.setRangingFrequency(1))
    {
        Serial.println(F("Cannot set ranging frequency requested. Aborting..."));
        return -1;
    }


    // set integration time
    if (myImager.setIntegrationTime(1000))
        Serial.println("Current integration time: " + String(myImager.getIntegrationTime()) + "ms");
    else {
        Serial.println(F("Cannot set integration time. Aborting..."));
        return -1;
    }

    Serial.println("start ranging...");

    // take measurement and go to sleep afterwards
    myImager.startRanging();
    int num_measurements = 0;

    while(num_measurements < NUM_MEASUREMENTS) {
        if (myImager.isDataReady() == true) {
            if (myImager.getRangingData(&measurementData)) {
                for (int i = 0 ; i < imageResolution; i++) {
                    int result = measurementData.distance_mm[i];
                    sensor_data[i / imageWidth][i % imageWidth] = result;
                    signal_strength[i / imageWidth][i % imageWidth] = measurementData.signal_per_spad[i];
                    sensor_status[i / imageWidth][i % imageWidth] = measurementData.target_status[i];
                }

                num_measurements++;
            }
        }
        delay(5);
    }

    myImager.stopRanging();

    Serial.println("finished measurement.");

    if(!myImager.setPowerMode(SF_VL53L5CX_POWER_MODE::SLEEP))
		Serial.print("vl53l5cx_set_power_mode failed\n");

    Serial.println("\n Sensor data:");

    // prepare sensor_data for image processing
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (sensor_status[i][j] == 5 || sensor_status[i][j] == 6 || sensor_status[i][j] == 9)
                processed_data[i][j] = sensor_data[i][j];
            else
                processed_data[i][j] = TANK_LVL_EMPTY_CM * 10;
            Serial.print("\t");
            Serial.print(sensor_data[i][j]);
        }
        Serial.println();
    }

    // print signal strength
    Serial.println("\n Signal strength: ");
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Serial.print("\t");
            Serial.print(signal_strength[i][j]);
        }
        Serial.println();
    }

    // print sensor status
    Serial.println("\n Sensor status: ");
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Serial.print("\t");
            Serial.print(sensor_status[i][j]);
        }
        Serial.println();
    }

    // Find minimum and maximum values in processed_data
    int min_value = INT_MAX;
    int max_value = 0;
    
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



    // Perform edge detection on padded_data
    _convolution(padded_data[0], kernel_edge, 10, 10, 3, 3);

    Serial.println("\n After egde detection: ");
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            Serial.print("\t");
            Serial.print(padded_data[i][j]);
        }
        Serial.println();
    }


    // crop back to sensor data size and copy the result of convolution back to processed_data.
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            processed_data[i][j] = padded_data[i + 1][j + 1];
        }
    }

    // populate holes in T shape with values using blur
    _convolution(processed_data[0], kernel_blur, 8, 8, 3, 3);

    // threshold it
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            processed_data[i][j] = processed_data[i][j] < 1 ? 0 : 1;
        }
    }

    Serial.println("\n After cropping, burring and thresholding: ");
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Serial.print("  ");
            Serial.print(processed_data[i][j]);
        }
        Serial.println();
    }



    // Perform pattern matching on processed_data. returns the intersection coordinates and best angle
    int intersection[2] = {0, 0};
    int best_angle = 0;
    int matching_score = 0;
    _patternMatching(processed_data, 8, 8, 15, 15, intersection, &best_angle, &matching_score);

    Serial.print("Intersection: "); Serial.print(intersection[0]); Serial.print(", "); Serial.println(intersection[1]);

    int x_1 = int(cos(radians(best_angle + 180)) * 4);
    int y_1 = int(sin(radians(best_angle + 180)) * 4);

    int x_2 = int(cos(radians(best_angle + 45)) * 4);
    int y_2 = int(sin(radians(best_angle + 45)) * 4);

    int x_3 = int(cos(radians(best_angle - 45)) * 4);
    int y_3 = int(sin(radians(best_angle - 45)) * 4);

    Serial.print("x_1: "); Serial.print(x_1); Serial.print(", y_1: "); Serial.println(y_1);
    Serial.print("x_2: "); Serial.print(x_2); Serial.print(", y_2: "); Serial.println(y_2);
    Serial.print("x_3: "); Serial.print(x_3); Serial.print(", y_3: "); Serial.println(y_3);

    Serial.println("score: " + String(matching_score));

    int measure_at[3][2] = {
        {intersection[0] + x_1, intersection[1] + y_1},
        {intersection[0] + x_2, intersection[1] + y_2},
        {intersection[0] + x_3, intersection[1] + y_3}
    };

    int measurements_chamber_large[20] = {0};
    int measurements_chamber_right[20] = {0};
    int measurements_chamber_left[20] = {0};
    int measurements_total[64] = {0};

    int index_chamber_large = 0;
    int index_chamber_right = 0;
    int index_chamber_left = 0;
    int index_measurements_total = 0;

    char chambers[8][8] = {0};
    memset(chambers, '.', sizeof(chambers));

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (!(sensor_status[i][j] == 5 || sensor_status[i][j] == 6 || sensor_status[i][j] == 9))
                continue; // we skip the point if it is not a valid measurement

            measurements_total[index_measurements_total++] = sensor_data[i][j];

            // Translate to origin (intersection)
            double x_sensor_frame = i - intersection[0];
            double y_sensor_frame = j - intersection[1];

            // Rotate around intersection by best_angle
            double radians = (best_angle + 90) * M_PI / 180.0 * -1;
            double cosA = cos(radians);
            double sinA = sin(radians);

            double x_t_frame = x_sensor_frame * cosA - y_sensor_frame * sinA;
            double y_t_frame = x_sensor_frame * sinA + y_sensor_frame * cosA;


            // Ensure the coordinates are within bounds
            double distance_from_center = sqrt(x_t_frame * x_t_frame + y_t_frame * y_t_frame);
            if (distance_from_center > 8) {
                continue; // we cancel early if the point is outside the circle
            }

            float scaling_factor = 1.5;

            bool x_in_chamber_large = true;
            bool y_in_chamber_large = y_t_frame >= scaling_factor;

            if (x_in_chamber_large && y_in_chamber_large) {
                measurements_chamber_large[index_chamber_large++] = sensor_data[i][j];
                chambers[i][j] = 'T';
            }

            bool x_in_chamber_right = x_t_frame >= scaling_factor;
            bool y_in_chamber_right = y_t_frame <= -scaling_factor;

            if (x_in_chamber_right && y_in_chamber_right) {
                measurements_chamber_right[index_chamber_right++] = sensor_data[i][j];
                chambers[i][j] = 'R';
            }

            bool x_in_chamber_left = x_t_frame <= -scaling_factor;
            bool y_in_chamber_left = y_t_frame <= -scaling_factor;

            if (x_in_chamber_left && y_in_chamber_left) {
                measurements_chamber_left[index_chamber_left++] = sensor_data[i][j];
                chambers[i][j] = 'L';
            }
        }
    }

    int median_chamber_large = calculate_median(measurements_chamber_large, index_chamber_large);
    int median_chamber_right = calculate_median(measurements_chamber_right, index_chamber_right);
    int median_chamber_left = calculate_median(measurements_chamber_left, index_chamber_left);

    Serial.println("Median in large compartement: " + String(median_chamber_large) + " mm");
    Serial.println("Median in small right compartement: " + String(median_chamber_right) + " mm");
    Serial.println("Median in small left compartement: " + String(median_chamber_left) + " mm");

    // Adjust coordinates if they are outside the size of sensor_data
    for (int i = 0; i < 3; i++) {
        measure_at[i][0] = max(0, min(measure_at[i][0], 7));
        measure_at[i][1] = max(0, min(measure_at[i][1], 7));
    }

    int measurements[3] = {0};
    for (int i = 0; i < 3; i++) {
        measurements[i] = sensor_data[measure_at[i][0]][measure_at[i][1]];
    }

    char points[3] = {'A', 'B', 'C'};
    Serial.println("Singe point measure at: ");
    for (int i = 0; i < 3; i++) {
        Serial.print("\t");
        Serial.print(points[i]);
        Serial.print(": ");
        Serial.print(measure_at[i][0]);
        Serial.print(", ");
        Serial.print(measure_at[i][1]);
        Serial.print(" -> ");
        Serial.println(measurements[i]);
    }


    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Serial.print("  ");
            if (i == intersection[0] && j == intersection[1]) {
                Serial.print("X");
            } else if (i == measure_at[0][0] && j == measure_at[0][1]) {
                Serial.print("A");
            } else if (i == measure_at[1][0] && j == measure_at[1][1]) {
                Serial.print("B");
            } else if (i == measure_at[2][0] && j == measure_at[2][1]) {
                Serial.print("C");
            } else {
                Serial.print(chambers[i][j]);
            }
        }
        Serial.println();
    }


    Serial.println("using median values");
    measurements[0] = median_chamber_large;
    measurements[1] = median_chamber_right;
    measurements[2] = median_chamber_left;

    int tank_level_cm = 30 + ((measurements[0] / 10 - 30) / 2)
                        + ((measurements[1] / 10 - 30) / 4)
                        + ((measurements[2] / 10 - 30) / 4);

    if (matching_score < 40) {
        Serial.println("confidence level too low! using general median.");
        tank_level_cm = calculate_median(measurements_total, index_measurements_total) / 10;
    }

    Serial.println("Tank level: " + String(tank_level_cm) + " cm");

    return tank_level_cm;
}


void _convolution(int* input, int kernel[3][3], int input_rows, int input_cols, int kernel_rows, int kernel_cols) {

    int temp[input_rows][input_cols] = {0};
    for (int i = 0; i < input_rows; i++) {
        for (int j = 0; j < input_cols; j++) {
            temp[i][j] = input[i * input_cols + j];
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
            input[i * input_cols + j] = convolute_result;
        }
    }
}



void _patternMatching(int input[8][8], int input_rows, int input_cols, int pattern_rows, int pattern_cols, int* intersection, int* best_angle, int* highest_score) {
    *highest_score = 0;
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
                if (convolute_result > *highest_score) {
                    *highest_score = convolute_result;
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
    double radians = angle * M_PI / 180.0 * -1; // needs to be for some reason, other wise rotates clockwise
    double cosA = cos(radians);
    double sinA = sin(radians);
    
    double centerX = (PATTERN_SIZE - 1) / 2.0;
    double centerY = (PATTERN_SIZE - 1) / 2.0;

    for (int x = 0; x < PATTERN_SIZE; x++) {
        for (int y = 0; y < PATTERN_SIZE; y++) {
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