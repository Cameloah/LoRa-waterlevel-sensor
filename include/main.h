#pragma once
#include <Arduino.h>
#include "lora_config.h"
#include "LoRa_E220.h"

#include "pin_config.h"
#include "display_tft.h"


#define TANK_DIAMETER_CM                                  200 // in cm
#define TANK_LVL_EMPTY_CM                                 400 // in cm
#define TANK_LVL_FULL_CM                                  20 // in cm
#define TANK_LITERS_RESERVE                               500 // in liters

#define MEASUREMENT_INTERVAL                              300000 // in milliseconds
#define RESPONSE_TIMEOUT                                  2000 + 3000 // 2s need for wakeup, + timout in milliseconds



// ---------------------- DATA ---------------------- //

typedef enum{
    SENSOR_ERROR_NONE           = 0x00,
    SENSOR_ERROR_TIMEOUT        = 0x01,
    SENSOR_ERROR_INVALID_DATA   = 0x02,
    SENSOR_ERROR_SEND           = 0x03,
    SENSOR_ERROR_RECEIVE        = 0x04,
    SENSOR_ERROR_ID             = 0x05,
    SENSOR_ERROR_UNKNOWN        = 0xFF
} SENSOR_ERROR_t;


typedef struct {
  int tank_id;
  int tank_measurement;
  int battery_voltage;
} tankData_t;

typedef struct {
  uint8_t address_L;
  int sensor_id;
  tankData_t data;
  int volume_liters;
  int volume_percent;
  int full_liters;
  SENSOR_ERROR_t error;
} remoteSensor_t;

extern int calculate_liters(int measurement);