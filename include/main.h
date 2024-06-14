#pragma once
#include <Arduino.h>



#define TANK_DIAMETER_CM                                  200 // in cm
#define TANK_LVL_EMPTY_CM                                 400 // in cm
#define TANK_LVL_FULL_CM                                  20 // in cm



extern int calculate_liters(int measurement);